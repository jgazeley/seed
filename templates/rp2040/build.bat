@echo off
REM Check if the "build" folder exists; if not, create it.
if not exist "build" (
    mkdir build
)

REM Change to the build folder.
cd build

echo Running cmake...
cmake -G "NMake Makefiles" ..

echo Running nmake...
nmake

pause
