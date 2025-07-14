@echo off
echo Setting up SDL2 development environment...

REM Check if vcpkg is available
where vcpkg >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Installing SDL2 packages via vcpkg...
    vcpkg install sdl2:x64-windows
    vcpkg install sdl2-image:x64-windows
    vcpkg install sdl2-ttf:x64-windows
    vcpkg install sdl2-mixer:x64-windows
    echo SDL2 packages installed!
) else (
    echo vcpkg not found. Please install SDL2 development libraries manually.
    echo Download from: https://github.com/libsdl-org/SDL/releases
    echo Extract headers to: include/SDL2/
    echo Extract .lib files to: lib/
)

echo Setup complete!
pause
