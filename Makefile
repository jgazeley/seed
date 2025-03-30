# nmake Makefile for seed project
# Assumes seed.c is in the project root
# and outputs object and exe files into the build directory.

# Directories and filenames
BUILD_DIR = build
SRC_DIR = src
SRC = $(SRC_DIR)\seed.c
OBJ = $(BUILD_DIR)\seed.obj
EXE = $(BUILD_DIR)\seed.exe

# Compiler and flags (adjust as necessary)
CC = cl.exe
CFLAGS = /O2 /nologo /W3

# Default target builds the executable
all: $(EXE)

# Link object file to create the executable
$(EXE): $(OBJ)
	$(CC) $(OBJ) /Fe$(EXE)

# Compile the source into the object file
$(OBJ): $(SRC)
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	$(CC) $(CFLAGS) /Fo$(BUILD_DIR)\ /c $(SRC)

# Clean up build artifacts
clean:
	del /q $(BUILD_DIR)\*.obj $(BUILD_DIR)\*.exe
