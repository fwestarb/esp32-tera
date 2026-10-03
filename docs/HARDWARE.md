# Hardware

Alvo: TTGO/LILYGO T-Display original, ESP32, tela ST7789 240×135 na orientação horizontal, flash usual de 4 MB, sem PSRAM. [Documentação LILYGO](https://github.com/Xinyuan-LilyGO/documentation/blob/master/en/products/t-display-series/t-display/index.md).

| Função | GPIO |
| --- | --- |
| Tela MOSI / clock | 19 / 18 |
| Tela CS / DC / reset / backlight | 5 / 16 / 23 / 4 |
| Botão de fala planejado | 35 |
| Botão de menu | 0 |
| Teclado I²C SDA / SCL | 21 / 22 |

GPIO35 é somente entrada e não tem pull-up interno; usar a polarização já existente na placa e confirmar versão. GPIO0 influencia o boot; não segurar ao reiniciar. Não reutilizar pinos da tela para áudio.

A imagem de referência corresponde ao **M5Stack CardKB**, teclado de 88×54 mm com I²C em `0x5F`, e não ao teclado integrado do Cardputer. [Fabricante](https://docs.m5stack.com/en/unit/Unit_CardKB).

Antes de conectar o teclado, confirmar alimentação e níveis de SDA/SCL. GPIO do ESP32 não tolera 5 V; usar conversão de nível se o módulo puxar sinais para 5 V. Não definir uma ligação de alimentação final apenas pela aparência da peça.

Áudio pendente: identificar microfone, saída/amplificador e interfaces (I²S ou analógica), alimentação e níveis. Nenhum pino de áudio foi atribuído. Captura de 5 s PCM mono 16 kHz/16 bits exige cerca de 160 kB; base64 aumenta para ~213 kB, sem contar TLS/JSON/display. Medir heap real e preferir fluxo/buffers pequenos em vez de guardar toda a resposta de áudio.
