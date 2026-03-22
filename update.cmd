@echo off
:start
echo For testing purposes only!
xcopy /y /-i "%~dp0\x64\Release\VietTypeATL64.dll" "%ProgramFiles%\VietType\VietTypeATL64.dll"
if %errorlevel% neq 0 goto :failed
xcopy /y /-i "%~dp0\x64\Release\VietTypeATL64.pdb" "%ProgramFiles%\VietType\VietTypeATL64.pdb"
if %errorlevel% neq 0 goto :failed
xcopy /y /-i "%~dp0\Win32\Release\VietTypeATL32.dll" "%ProgramFiles(x86)%\VietType\VietTypeATL32.dll"
if %errorlevel% neq 0 goto :failed
xcopy /y /-i "%~dp0\Win32\Release\VietTypeATL32.pdb" "%ProgramFiles(x86)%\VietType\VietTypeATL32.pdb"
if %errorlevel% neq 0 goto :failed

echo Update successful.
pause
exit /b 0

:failed
echo.
echo Update failed!
set /p retry="Do you want to try again? (Y/N): "
if /i "%retry%"=="Y" goto :start

exit /b 1
