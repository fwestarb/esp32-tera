@echo off
cd /d "%~dp0"
python tools\install_pc.py
if errorlevel 1 goto failed
python tools\install_dora.py
if errorlevel 1 goto failed
echo Instalacao concluida. Abra configurar.cmd.
pause
exit /b 0
:failed
echo Nao foi possivel concluir. Confira a mensagem acima e o guia docs\INSTALACAO-ALPHA.md.
pause
exit /b 1
