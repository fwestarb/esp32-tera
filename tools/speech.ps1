$ErrorActionPreference = 'Stop'
[Console]::InputEncoding = New-Object System.Text.UTF8Encoding($false)
[Console]::OutputEncoding = New-Object System.Text.UTF8Encoding($false)
$teraSynth = $null
try {
    $teraRequest = [Console]::In.ReadToEnd() | ConvertFrom-Json
    if (-not ($teraRequest.text -is [string]) -or $teraRequest.text.Length -lt 1 -or $teraRequest.text.Length -gt 4000) { throw 'Texto de fala invalido.' }
    Add-Type -AssemblyName System.Speech
    $teraSynth = New-Object System.Speech.Synthesis.SpeechSynthesizer
    $teraInstalled = @($teraSynth.GetInstalledVoices() | Where-Object { $_.Enabled })
    $teraPortuguese = @($teraInstalled | Where-Object { $_.VoiceInfo.Culture.Name -eq 'pt-BR' })
    if ($teraPortuguese.Count -gt 0) { $teraSynth.SelectVoice($teraPortuguese[0].VoiceInfo.Name) }
    $teraSynth.Rate = 0
    $teraSynth.Volume = 80
    if ($teraRequest.wav) { $teraSynth.SetOutputToWaveFile([string]$teraRequest.wav) }
    else { $teraSynth.SetOutputToDefaultAudioDevice() }
    $teraClock = [System.Diagnostics.Stopwatch]::StartNew()
    $teraSynth.Speak([string]$teraRequest.text)
    $teraClock.Stop()
    @{ok=$true; voice=$teraSynth.Voice.Name; culture=$teraSynth.Voice.Culture.Name; characters=$teraRequest.text.Length; elapsedMs=$teraClock.ElapsedMilliseconds; mode=$(if($teraRequest.wav){'wav'}else{'playback'})} | ConvertTo-Json -Compress
} catch {
    @{ok=$false; error=$_.Exception.Message} | ConvertTo-Json -Compress
    exit 1
} finally { if ($teraSynth) { $teraSynth.Dispose() } }
