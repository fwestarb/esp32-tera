# Hardware

Alvo: TTGO/LILYGO T-Display original, ESP32, tela ST7789 240×135 na orientação horizontal, flash usual de 4 MB, sem PSRAM. [Documentação LILYGO](https://github.com/Xinyuan-LilyGO/documentation/blob/master/en/products/t-display-series/t-display/index.md).

| Função | GPIO |
| --- | --- |
| Tela MOSI / clock | 19 / 18 |
| Tela CS / DC / reset / backlight | 5 / 16 / 23 / 4 |
| Botão de fala | 35 |
| Botão de menu | 0 |
| Entrada do microfone analógico | 32 (ADC1) |
| Teclado I²C opcional, ainda não implementado | 21 / 22 reservados |

GPIO35 é somente entrada e não tem pull-up interno; usar a polarização já existente na placa e confirmar versão. GPIO0 influencia o boot; não segurar ao reiniciar. Não reutilizar pinos da tela para áudio.

A imagem de referência corresponde ao **M5Stack CardKB**, teclado de 88×54 mm com I²C em `0x5F`, e não ao teclado integrado do Cardputer. [Fabricante](https://docs.m5stack.com/en/unit/Unit_CardKB).

Antes de conectar o teclado, confirmar alimentação e níveis de SDA/SCL. GPIO do ESP32 não tolera 5 V; usar conversão de nível se o módulo puxar sinais para 5 V. Não definir uma ligação de alimentação final apenas pela aparência da peça.

## Microfone atual

A montagem validada pelo usuário usa cápsula de eletreto e dois estágios NPN alimentados em 3,3 V. A saída está no **GPIO32**, com offset medido de 1,644 V. Não é GPIO34. A cápsula precisa de polarização e pré-amplificação; não conecte uma cápsula nua esperando o mesmo resultado.

Cada estágio usa divisor de base 47 kΩ para 3V3 e 22 kΩ para GND, emissor com 1 kΩ para GND e coletor com 3,9 kΩ para 3V3. A cápsula tem resistor de polarização de 2,2 kΩ. Acoplamento de entrada 1 µF; entre estágios 10 µF; saída 10 µF para OUT, com dois resistores de 100 kΩ formando offset entre 3V3 e GND. Desacoplamento da alimentação: 100 nF e 10 µF adicionais. A polaridade dos eletrolíticos depende das tensões DC medidas; confirme antes de ligar. Pinagem de transistor e polaridade da cápsula variam conforme o componente.

Medições informadas: MIC 2,974 V; primeiro transistor B/E/C 1,027/0,425/1,650 V; segundo B/E/C 0,916/0,321/2,048 V; OUT 1,644 V. São referências desta montagem, não valores garantidos para qualquer componente. Meça OUT antes de conectar ao ESP32 e mantenha-o entre 0 e 3,3 V. Compartilhe GND, use fios curtos e não aplique 5 V ao GPIO.

O firmware captura aproximadamente 8 kHz por software, até dez segundos. Mantém PCM8 compacto durante a transmissão como WAV. Não há teclado nem saída de áudio física implementados. A voz de saída é fornecida pelos alto-falantes do computador com o aplicativo ativo. Gravações anteriores reais e comandos sintetizados são usados nos testes; não equivalem a uma nova captura física de fala.
