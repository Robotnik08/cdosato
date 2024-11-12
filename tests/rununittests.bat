:: Run all .to files in the tests, if any return non-zero, unit tests failed
:: This script is used to run all unit tests in the tests using the compiled ./main.exe

@echo off
@setlocal

set start=%time%

for %%f in (*.to) do (
    echo Running %%f
    powershell ../build/dosato %%f
    if errorlevel 1 (
        echo Unit tests failed
        echo At: %%f
        goto :end
    )
)

set end=%time%

echo All unit tests passed
echo Start: %start%
echo End: %end%

:end

@echo on