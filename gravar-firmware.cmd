@echo off
cd /d "%~dp0"
if not exist .venv-pc\Scripts\python.exe (
  echo Primeiro execute instalar.cmd. Encerre o aplicativo Tera antes de gravar.
  pause
  exit /b 1
)
set /p TERA_PORT=Porta da TTGO, por exemplo COM3: 
.venv-pc\Scripts\python.exe tools\flash_alpha.py --port "%TERA_PORT%"
pause
