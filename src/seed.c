#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shlwapi.h>   // For PathRemoveFileSpecA and PathFileExistsA
#pragma comment(lib, "Shlwapi.lib")

#define MAX_PATH_LEN 1024

// -----------------------------------------------------------------------------
// Helper: Replace all occurrences of 'placeholder' in 'src' with 'replacement'.
// Returns a newly allocated string that must be freed by the caller.
char* replace_placeholder(const char* src, const char* placeholder, const char* replacement) {
    size_t src_len = strlen(src);
    size_t ph_len = strlen(placeholder);
    size_t rep_len = strlen(replacement);
    size_t count = 0;
    const char* tmp = src;
    while ((tmp = strstr(tmp, placeholder)) != NULL) {
        count++;
        tmp += ph_len;
    }
    size_t new_len = src_len + count * (rep_len - ph_len) + 1;
    char* result = (char*)malloc(new_len);
    if (!result) return NULL;
    result[0] = '\0';

    const char* pos = src;
    char* out = result;
    size_t remaining = new_len; // remaining buffer size
    while (1) {
        const char* p = strstr(pos, placeholder);
        if (!p) {
            // Copy the rest of the string.
            if (strcpy_s(out, remaining, pos) != 0) {
                free(result);
                return NULL;
            }
            break;
        }
        size_t len = p - pos;
        // Copy the part before the placeholder.
        if (len > 0) {
            if (strncpy_s(out, remaining, pos, len) != 0) {
                free(result);
                return NULL;
            }
            out += len;
            remaining -= len;
        }
        // Copy the replacement string.
        if (strcpy_s(out, remaining, replacement) != 0) {
            free(result);
            return NULL;
        }
        out += rep_len;
        remaining -= rep_len;
        pos = p + ph_len;
    }
    return result;
}

// -----------------------------------------------------------------------------
// Helper: Read entire file into a newly allocated string.
// Returns the file content on success or NULL on error. The caller must free the returned string.
char* read_file(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Could not open file %s for reading.\n", filename);
        return NULL;
    }
    // Move to end of file to determine its length.
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    // Allocate buffer for file content plus null terminator.
    char* buffer = (char*)malloc(len + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed for file %s.\n", filename);
        fclose(f);
        return NULL;
    }
    size_t read_len = fread(buffer, 1, len, f);
    buffer[read_len] = '\0';  // Null-terminate the string.
    fclose(f);
    return buffer;
}

// -----------------------------------------------------------------------------
// Helper: Write content to a file.
int write_file(const char* filename, const char* content) {
    FILE* f = fopen(filename, "wb");
    if (!f) return 1;
    size_t written = fwrite(content, 1, strlen(content), f);
    fclose(f);
    return (written == strlen(content)) ? 0 : 1;
}

// -----------------------------------------------------------------------------
// Copies a file from srcFile to destFile. If 'substitute' is nonzero,
// treats the file as text and substitutes the placeholder "${PROJECT_NAME}"
// with the provided projectName. 
// Also, if the file's name (ignoring case) is "blink.c", renames it to <projectName>.c.
int copy_file_with_substitution(const char* srcFile, const char* destFile, const char* projectName, int substitute, const char* originalName) {
    char finalDest[MAX_PATH_LEN];

    // If this file is "blink.c", build a new destination file name.
    if (originalName && _stricmp(originalName, "blink.c") == 0) {
        char destDir[MAX_PATH_LEN];
        // Copy destFile (which is assumed to be a full path including filename)
        if (strcpy_s(destDir, sizeof(destDir), destFile) != 0) {
            fprintf(stderr, "Error copying destination path.\n");
            return 1;
        }
        // Remove the file component to leave the directory.
        if (!PathRemoveFileSpecA(destDir)) {
            fprintf(stderr, "Error: Unable to get destination directory from %s\n", destFile);
            return 1;
        }
        // Build new destination filename as <destDir>\<projectName>.c
        snprintf(finalDest, sizeof(finalDest), "%s\\%s.c", destDir, projectName);
    } else {
        // Otherwise, use destFile as given.
        if (strcpy_s(finalDest, sizeof(finalDest), destFile) != 0) {
            fprintf(stderr, "Error copying destination path.\n");
            return 1;
        }
    }

    if (substitute) {
        char* content = read_file(srcFile);
        if (!content) {
            fprintf(stderr, "Error reading file: %s\n", srcFile);
            return 1;
        }
        char* newContent = replace_placeholder(content, "${PROJECT_NAME}", projectName);
        free(content);
        if (!newContent) {
            fprintf(stderr, "Error processing substitution in file: %s\n", srcFile);
            return 1;
        }
        int ret = write_file(finalDest, newContent);
        free(newContent);
        return ret;
    } else {
        if (!CopyFileA(srcFile, finalDest, FALSE)) {
            fprintf(stderr, "Error copying file: %s to %s (error %lu)\n", srcFile, finalDest, GetLastError());
            return 1;
        }
    }
    return 0;
}

// -----------------------------------------------------------------------------
// Determines if a file is considered text based on its extension.
int is_text_file(const char* filename) {
    // Check for Makefile (case insensitive)
    if (_stricmp(filename, "Makefile") == 0)
        return 1;
    
    const char* ext = strrchr(filename, '.');
    if (!ext) return 0;
    
    if (_stricmp(ext, ".c") == 0 || _stricmp(ext, ".h") == 0 ||
        _stricmp(ext, ".txt") == 0 || _stricmp(ext, ".cfg") == 0 ||
        _stricmp(ext, ".cmake") == 0 || _stricmp(ext, ".ps1") == 0 ||
        _stricmp(ext, ".bat") == 0) {
        return 1;
    }
    
    return 0;
}

// -----------------------------------------------------------------------------
// Recursively copies a directory from srcDir to destDir, performing substitution
// on text files. 'projectName' is used to replace "${PROJECT_NAME}" in text files.
// Files named "blink.c" are renamed to <projectName>.c.
int copy_directory(const char* srcDir, const char* destDir, const char* projectName) {
    WIN32_FIND_DATAA findData;
    char searchPath[MAX_PATH_LEN];
    snprintf(searchPath, sizeof(searchPath), "%s\\*", srcDir);
    HANDLE hFind = FindFirstFileA(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: Could not open directory: %s\n", srcDir);
        return 1;
    }
    // Create destination directory.
    if (!CreateDirectoryA(destDir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        fprintf(stderr, "Error: Could not create directory: %s\n", destDir);
        FindClose(hFind);
        return 1;
    }
    do {
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
            continue;
        char srcPath[MAX_PATH_LEN], dstPath[MAX_PATH_LEN];
        snprintf(srcPath, sizeof(srcPath), "%s\\%s", srcDir, findData.cFileName);
        snprintf(dstPath, sizeof(dstPath), "%s\\%s", destDir, findData.cFileName);
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (copy_directory(srcPath, dstPath, projectName) != 0) {
                FindClose(hFind);
                return 1;
            }
        } else {
            int text = is_text_file(findData.cFileName);
            // If the file is "blink.c", we want to rename it.
            if (_stricmp(findData.cFileName, "blink.c") == 0) {
                snprintf(dstPath, sizeof(dstPath), "%s\\%s.c", destDir, projectName);
            }
            if (copy_file_with_substitution(srcPath, dstPath, projectName, text, findData.cFileName) != 0) {
                FindClose(hFind);
                return 1;
            }
        }
    } while (FindNextFileA(hFind, &findData));
    FindClose(hFind);
    return 0;
}

// -----------------------------------------------------------------------------
// Platform-specific generator functions.
// For each platform, the template is copied from <exe_dir>\templates\<platform>
// to the new project folder.
int generate_rp2040_template(const char* projectName, const char* templateDir) {
    return copy_directory(templateDir, projectName, projectName);
}

int generate_atmega328p_template(const char* projectName, const char* templateDir) {
    return copy_directory(templateDir, projectName, projectName);
}

int generate_atmega2560_template(const char* projectName, const char* templateDir) {
    return copy_directory(templateDir, projectName, projectName);
}

int generate_pic18_template(const char* projectName, const char* templateDir) {
    return copy_directory(templateDir, projectName, projectName);
}

// -----------------------------------------------------------------------------
// Main dispatcher.
int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: seed <platform> <projectName>\n");
        return 1;
    }
    const char* platform = argv[1];
    const char* projectName = argv[2];

    // Get the full path of seed.exe.
    char exePath[MAX_PATH_LEN];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH_LEN) == 0) {
        fprintf(stderr, "Error: Unable to get executable path.\n");
        return 1;
    }
    // Remove the executable name; now exePath is the build folder where seed.exe resides.
    if (!PathRemoveFileSpecA(exePath)) {
        fprintf(stderr, "Error: Unable to get executable directory.\n");
        return 1;
    }

    // Now, seed.exe is in the build folder, so get its parent directory (the project root).
    char parentDir[MAX_PATH_LEN];
    if (strcpy_s(parentDir, sizeof(parentDir), exePath) != 0) {
        fprintf(stderr, "Error: Unable to copy exePath.\n");
        return 1;
    }
    if (!PathRemoveFileSpecA(parentDir)) {
        fprintf(stderr, "Error: Unable to get parent directory.\n");
        return 1;
    }
    
    // Construct the template directory: <parentDir>\templates\<platform>
    char templateDir[MAX_PATH_LEN];
    snprintf(templateDir, sizeof(templateDir), "%s\\templates\\%s", parentDir, platform);
    if (!PathFileExistsA(templateDir)) {
        fprintf(stderr, "Error: Template directory not found: %s\n", templateDir);
        return 1;
    }

    int res = 0;
    if (_stricmp(platform, "rp2040") == 0) {
        res = generate_rp2040_template(projectName, templateDir);
    } else if (_stricmp(platform, "atmega328p") == 0) {
        res = generate_atmega328p_template(projectName, templateDir);
    } else if (_stricmp(platform, "atmega2560") == 0) {
        res = generate_atmega2560_template(projectName, templateDir);
    } else if (_stricmp(platform, "18f1320") == 0) {
        res = generate_pic18_template(projectName, templateDir);
    } else {
        fprintf(stderr, "Error: Unknown platform '%s'\n", platform);
        return 1;
    }
    if (res == 0) {
        printf("Project '%s' for platform '%s' generated successfully.\n", projectName, platform);
    } else {
        fprintf(stderr, "Project generation failed!!\n");
        return 1;
    }
    return 0;
}