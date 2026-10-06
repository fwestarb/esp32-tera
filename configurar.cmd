@echo off
cd /d "%~dp0"
python tools\tera_usb.py
if errorlevel 1 pause
