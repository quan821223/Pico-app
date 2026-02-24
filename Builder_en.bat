@echo off
chcp 65001 > nul
setlocal enabledelayedexpansion

:: ===========================================
:: Raspberry Pi Pico Auto Build Script
:: ===========================================

title Pico Auto Build Tool

:: Set color (Green text)
color 0A

echo.
echo ========================================
echo   Raspberry Pi Pico Auto Build Tool
echo ========================================
echo.

:: Check if CMakeLists.txt exists
if not exist "CMakeLists.txt" (
    color 0C
    echo [ERROR] CMakeLists.txt not found
    echo Please run this script in your Pico project root directory
    pause
    exit /b 1
)

:: Step 1: Clean old build folder
echo [Step 1/4] Cleaning old build folder...
if exist "build" (
    echo Deleting build folder...
    rmdir /s /q build
    if errorlevel 1 (
        color 0C
        echo [ERROR] Failed to delete build folder
        pause
        exit /b 1
    )
    echo Deletion complete
) else (
    echo Build folder does not exist, skipping
)
echo.

:: Step 2: Create new build folder
echo [Step 2/4] Creating new build folder...
mkdir build
if errorlevel 1 (
    color 0C
    echo [ERROR] Failed to create build folder
    pause
    exit /b 1
)
echo Creation complete
echo.

:: Step 3: Run CMake configuration
echo [Step 3/4] Running CMake configuration...
cd build
cmake -G "NMake Makefiles" ..
if errorlevel 1 (
    color 0C
    echo [ERROR] CMake configuration failed
    cd ..
    pause
    exit /b 1
)
echo CMake configuration complete
echo.

:: Step 4: Build project
echo [Step 4/4] Building project...
nmake
if errorlevel 1 (
    color 0C
    echo [ERROR] Build failed
    cd ..
    pause
    exit /b 1
)
echo.

:: Return to project root
cd ..

:: Build successful
color 0A
echo ========================================
echo   Build Successful!
echo ========================================
echo.

:: Find and display generated .uf2 files
echo Searching for build results...
for /r build %%f in (*.uf2) do (
    echo [SUCCESS] Found UF2 file:
    echo    %%f
    echo.
    
    :: Ask if copy to current directory
    set /p copy_choice=Copy UF2 file to current directory? (Y/N): 
    if /i "!copy_choice!"=="Y" (
        copy "%%f" .
        if errorlevel 1 (
            echo [WARNING] Copy failed
        ) else (
            echo [SUCCESS] Copied to current directory
        )
    )
    echo.
    goto found_uf2
)

echo [WARNING] No .uf2 file found
echo Please check the build directory for compilation results
echo.

:found_uf2

:: Display build information
echo ========================================
echo Build Information:
echo ----------------------------------------
echo Project Directory: %CD%
echo Build Directory: %CD%\build
echo Completion Time: %date% %time%
echo ========================================
echo.

:: Ask if open build folder
set /p open_choice=Open build folder? (Y/N): 
if /i "%open_choice%"=="Y" (
    start explorer build
)

echo.
echo Press any key to exit...
pause > nul