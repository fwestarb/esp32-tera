@echo off
cd /d "%~dp0"
if exist .venv-pc\Scripts\python.exe (
  .venv-pc\Scripts\python.exe tools\tera_usb.py
) else (
  python tools\tera_usb.py
)
if errorlevel 1 pause
