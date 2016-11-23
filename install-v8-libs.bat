@echo off
setlocal
for /f "tokens=*" %%i in ('where git.exe') do set GIT=%%i
FOR %%i IN ("%GIT%") DO (
  "%%~dpi..\bin\bash" install-v8-libs.sh
)
