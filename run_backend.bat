@echo off
cd /d "%~dp0"
echo Compiling PixelClear Studio C backend...
gcc backend\backend.c -o backend.exe -lws2_32
if errorlevel 1 (
  echo.
  echo Compile failed. Check that GCC/MinGW is installed and available in PATH.
  pause
  exit /b 1
)
echo.
echo Starting PixelClear Studio...
echo Open http://127.0.0.1:8000/ in your browser.
backend.exe
pause
