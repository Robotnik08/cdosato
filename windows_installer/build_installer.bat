@echo off
:: Build installer zip using WinRAR for Windows

:: Check if WinRAR is installed
where winrar >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo WinRAR is not installed. Please install WinRAR and try again.
    exit /b 1
)

:: Clean and rebuild the project
make -C ../ clean
if %ERRORLEVEL% NEQ 0 (
    echo Clean failed, continuing with build.
)

make -C ../
if %ERRORLEVEL% NEQ 0 (
    echo Build failed. Please fix the build errors and try again.
    exit /b 1
)

:: Define variables for paths
set OUTPUT_DIR=installer_output
set SFX_CONFIG=build_installer.sfx
set INSTALLER_NAME=dosato_installer.exe

:: Ensure the output directory exists
if not exist "%OUTPUT_DIR%" (
    mkdir "%OUTPUT_DIR%"
)

:: Copy required files to the output directory
copy /Y "..\build\dosato.exe" "%OUTPUT_DIR%\"
if %ERRORLEVEL% NEQ 0 (
    echo Failed to copy dosato.exe. Ensure the build folder exists and try again.
    exit /b 1
)

copy /Y "..\build\libdosato.dll" "%OUTPUT_DIR%\"
if %ERRORLEVEL% NEQ 0 (
    echo Failed to copy libdosato.dll. Ensure the build folder exists and try again.
    exit /b 1
)

copy /Y "..\dosato_libraries\dosato.h" "%OUTPUT_DIR%\"
if %ERRORLEVEL% NEQ 0 (
    echo Failed to copy dosato.h. Ensure the libraries folder exists and try again.
    exit /b 1
)

copy /Y "README.txt" "%OUTPUT_DIR%\"
if %ERRORLEVEL% NEQ 0 (
    echo Failed to copy README.txt. Ensure the file exists and try again.
    exit /b 1
)

copy /Y "setup.bat" "%OUTPUT_DIR%\"
if %ERRORLEVEL% NEQ 0 (
    echo Failed to copy setup.bat. Ensure the file exists and try again.
    exit /b 1
)

:: Create the installer with WinRAR
winrar a -r -ep1 -ibck -m5 -sfx -z"%SFX_CONFIG%" "%INSTALLER_NAME%" "%OUTPUT_DIR%\*"
if %ERRORLEVEL% NEQ 0 (
    echo Failed to create the installer. Please check the SFX configuration and files, then try again.
    exit /b 1
)

:: Notify the user of success
echo Installer created successfully: %INSTALLER_NAME%
exit /b 0
