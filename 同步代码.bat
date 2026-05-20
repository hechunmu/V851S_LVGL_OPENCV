@echo off
setlocal EnableExtensions

cd /d "%~dp0"

echo [INFO] Run pre-sync cleanup...


echo [INFO] Cleanup done

where git >nul 2>nul
if errorlevel 1 (
    echo [ERROR] Git not found in PATH
    pause
    exit /b 1
)

git rev-parse --is-inside-work-tree >nul 2>nul
if errorlevel 1 (
    echo [ERROR] Current folder is not a Git repository
    pause
    exit /b 1
)

for /f %%i in ('git branch --show-current') do set BRANCH=%%i
if not defined BRANCH set BRANCH=master

if /I not "%BRANCH%"=="master" (
    echo [ERROR] Current branch is %BRANCH%
    echo [ERROR] Please switch to master first
    pause
    exit /b 1
)

git remote get-url origin >nul 2>nul
if errorlevel 1 (
    echo [ERROR] Remote origin is not configured
    echo [INFO] Run this once:
    echo git remote add origin https://gitee.com/hechunmu/stc32-g_-bldc-bk-ai-ota.git
    pause
    exit /b 1
)

if "%~1"=="" (
    for /f %%i in ('powershell -NoProfile -Command "Get-Date -Format yyyy-MM-dd_HH-mm-ss"') do set MSG=sync %%i
) else (
    set MSG=%*
)

echo [INFO] Repo: %cd%
echo [INFO] Commit: %MSG%
echo [INFO] Start sync...

git add -A
if errorlevel 1 goto :fail

git diff --cached --quiet
if errorlevel 1 (
    git commit -m "%MSG%"
    if errorlevel 1 goto :fail
) else (
    echo [INFO] Nothing to commit
)

git pull --rebase origin master
if errorlevel 1 goto :fail

git push origin master
if errorlevel 1 goto :fail

echo [OK] Sync completed
pause
exit /b 0

:fail
echo [ERROR] Sync failed
pause
exit /b 1