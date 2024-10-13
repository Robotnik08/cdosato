:: Build installer zip using winRAR FOR WINDOWS

:: Check if winRAR is installed
where winrar >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo winRAR is not installed. Please install winRAR and try again.
    exit /b 1
)

make -C ../ clean
if %ERRORLEVEL% NEQ 0 (
    echo Clean failed. Please fix the build errors and try again.
    exit /b 1
)
make -C ../
if %ERRORLEVEL% NEQ 0 (
    echo Build failed. Please fix the build errors and try again.
    exit /b 1
)

:: Create installer zip

:: Files to contain:
::: - build/dosato.exe
::: - build/libdosato.dll
::: - dosato_libraries/dosato.h



:: Create installer zip
winrar a -r -ep1 -ibck -m5 -sfx -z"build_installer.sfx" "dosato_installer" "../build/dosato.exe" "../build/libdosato.dll" "../dosato_libraries/dosato.h" "README.txt"

if %ERRORLEVEL% NEQ 0 (
    echo Build failed. Please fix the build errors and try again.
    exit /b 1
)

echo Installer created successfully.