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

## Website

The website uses six React Bits components supplied in the author's attachments. They retain the [React Bits MIT + Commons Clause license](../website/src/components/LICENSE-REACT-BITS.md), separately from the project's own MIT source. Adaptations cover the Tera palette, PNG alpha, page scrolling and accessible labels. Components are integrated into this website, not distributed as a standalone component library.

| Component | Source | License |
|---|---|---|
| React / React DOM | https://github.com/facebook/react | MIT |
| React Bits components | https://github.com/DavidHDev/react-bits | MIT + Commons Clause, notice preserved |
| OGL | https://github.com/oframe/ogl | Unlicense |
| GSAP | https://gsap.com/standard-license/ | Standard no-charge license |
| Hugeicons React / free icons | https://github.com/hugeicons/hugeicons-react | MIT |
| Vite | https://github.com/vitejs/vite | MIT; build dependency |

Dappled Light is **not installed**; it needs an unavailable React Bits Pro license. Registry configuration references an environment variable only. Generated tree artwork is an illustration; the prototype photo was provided by the author. Fonts Oi and STIX Two Text are served by Google Fonts with local serif fallbacks. The current tree mark and three prototype photographs were supplied by the author on October 11, 2026.
