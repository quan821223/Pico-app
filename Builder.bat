@echo off
setlocal

set "BOARD=%~1"
if "%BOARD%"=="" set "BOARD=pico"

set "EXTRA_ARG="
if /i "%~2"=="elf-only" set "EXTRA_ARG=-ElfOnly"

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\build_firmware.ps1" -Board "%BOARD%" %EXTRA_ARG%
exit /b %ERRORLEVEL%
