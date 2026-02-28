@echo off
setlocal
set "SCRIPT_DIR=%~dp0"
set "GRADLE_USER_HOME=%SCRIPT_DIR%gradle-home"
call "%SCRIPT_DIR%gradlew.bat" --no-daemon --console=plain assembleDebug
if errorlevel 1 exit /b 1
taskkill /im ninja.exe /f >nul 2>&1
taskkill /im cmake.exe /f >nul 2>&1
rmdir /s /q "%SCRIPT_DIR%app\\.cxx\\RelWithDebInfo" >nul 2>&1
call "%SCRIPT_DIR%gradlew.bat" --no-daemon --console=plain assembleRelease
if errorlevel 1 exit /b 1

echo.
echo APKs written to app\build\outputs\apk
endlocal
