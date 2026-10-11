# Third-party components

ESP32 Tera source and generated firmware are distinct from third-party dependencies. Keep the upstream notices when redistributing their files. Voice models are downloaded separately, not bundled in the release ZIP.

| Component | Source | License / usage |
|---|---|---|
| Kokoro model and Dora voice | https://huggingface.co/hexgrad/Kokoro-82M | Apache-2.0; upstream model card describes training attribution |
| kokoro-onnx | https://github.com/thewh1teagle/kokoro-onnx | MIT |
| eSpeak NG, used by the voice engine | https://github.com/espeak-ng/espeak-ng | GPL-3.0; installed as a separate dependency |
| Optional Piper engine | https://github.com/OHF-Voice/piper1-gpl | GPL-3.0; installed separately |
| Optional Faber model | https://huggingface.co/rhasspy/piper-voices/tree/main/pt/pt_BR/faber/medium | Consult MODEL_CARD downloaded by the installer |
| Arduino ESP32 | https://github.com/espressif/arduino-esp32 | LGPL-2.1 and component-specific notices |
| TFT_eSPI | https://github.com/Bodmer/TFT_eSPI | FreeBSD |
| ArduinoJson 6 | https://github.com/bblanchon/ArduinoJson | MIT |
| esptool | https://github.com/espressif/esptool | GPL-2.0-or-later |
| PySerial | https://github.com/pyserial/pyserial | BSD-3-Clause |

Cloud services have their own terms, privacy policies and quotas. API credentials are personal and are not included in source or downloads.
