@echo off

REM Store the current directory
set NewPath=%cd%

REM Fetch the current PATH variable from the registry
for /f "tokens=2*" %%a in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path ^| findstr /i "Path"') do set CurrentPath=%%b

REM Normalize the PATH variable (remove quotes if any)
set CurrentPath=%CurrentPath:"=%

REM Check if the new path already exists
echo %CurrentPath% | find /i "%NewPath%" >nul
if %errorlevel%==0 (
    echo The path "%NewPath%" is already in the PATH variable.
    pause
    exit /b 0
)

REM Check if appending the new path exceeds the 1024-character limit
set CombinedPath=%CurrentPath%;%NewPath%
if not "%CombinedPath:~1024%"=="" (
    echo Error: PATH variable exceeds the 1024-character limit.
    pause
    exit /b 1
)

REM Append the new path and update the PATH variable
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path /t REG_EXPAND_SZ /d "%CombinedPath%" /f

REM Refresh the system's PATH variable in the current session
set PATH=%CombinedPath%

echo PATH updated successfully with "%NewPath%".

REM Create libraries folder inside the current directory
if not exist "libraries" (
    mkdir "libraries"
)

REM Add PATH variable for the libraries folder
REM Get new path
set NewPath=%cd%\libraries

REM Check if the new path already exists
echo %CurrentPath% | find /i "%NewPath%" >nul
if %errorlevel%==0 (
    echo The path "%NewPath%" is already in the PATH variable.
    pause
    exit /b 0
)

REM Check if appending the new path exceeds the 1024-character limit
set CombinedPath=%CombinedPath%;%NewPath%

if not "%CombinedPath:~1024%"=="" (
    echo Error: PATH variable exceeds the 1024-character limit.
    pause
    exit /b 1
)

REM Append the new path and update the PATH variable
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path /t REG_EXPAND_SZ /d "%CombinedPath%" /f

REM Refresh the system's PATH variable in the current session
set PATH=%CombinedPath%

echo PATH updated successfully with "%NewPath%".
pause

exit /b 0
