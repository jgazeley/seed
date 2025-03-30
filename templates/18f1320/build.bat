@echo off
REM Check if the "build" folder exists; if not, create it.
if not exist "build" (
    mkdir build
)

REM Change to the build folder.
cd build

echo Running xc8-cc...
xc8-cc -mcpu=18f1320 ../${PROJECT_NAME}.c
pause