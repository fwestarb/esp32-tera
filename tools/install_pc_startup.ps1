$ErrorActionPreference = 'Stop'
$teraProject = Split-Path -Parent $PSScriptRoot
$teraPython = (Get-Command pythonw.exe -ErrorAction Stop).Source
$teraStartup = [Environment]::GetFolderPath('Startup')
$teraShell = New-Object -ComObject WScript.Shell
$teraShortcut = $teraShell.CreateShortcut((Join-Path $teraStartup 'ESP32 Tera.lnk'))
$teraShortcut.TargetPath = $teraPython
$teraShortcut.Arguments = '"' + (Join-Path $PSScriptRoot 'tera_usb.py') + '" --minimized'
$teraShortcut.WorkingDirectory = $teraProject
$teraShortcut.Description = 'Assistente USB da ESP32 Tera: voz e comandos no PC'
$teraShortcut.WindowStyle = 7
$teraShortcut.Save()
Write-Output 'ESP32 Tera configurada para iniciar minimizada ao entrar no Windows.'
