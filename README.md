# ESP32 Tera

Chat de IA em desenvolvimento para a **TTGO T-Display original**. Entrada por voz ao segurar um botão e envio ao soltar; teclado M5Stack CardKB e saída de áudio opcionais. Após configurar, o dispositivo deverá operar com Wi-Fi sem computador ligado.

## Estado atual

A primeira etapa contém firmware base para display/botões, paleta Tera, árvore sem folhas, leitura de configurações NVS e planejamento. **Ainda não há chat, captura de áudio, reprodução, instalador ou driver de teclado funcional.** Não foi gravado ou validado em hardware.

## Compilar a base

Instale PlatformIO e execute na raiz:

```text
pio run
```

Para gravar, somente quando a placa estiver conectada e a porta confirmada:

```text
pio run --target upload --upload-port PORTA
pio device monitor --port PORTA
```

O firmware atual mostra uma tela inicial e alterna detalhes pelo botão GPIO0. A chave pessoal será configurada depois da gravação, sem editar o código. `config.example.json` é o contrato planejado; ainda não é importado pelo firmware.

## Planejamento

- [Sequência e critérios de conclusão](docs/PLANO.md)
- [Placa, teclado e áudio](docs/HARDWARE.md)
- [Pesquisa dos provedores](docs/PROVEDORES.md)

## English

An AI chat device under development for the original TTGO T-Display. Planned features: hold-to-talk/release-to-send, optional M5Stack CardKB keyboard, optional speech output, and standalone Wi-Fi operation after setup.

The current stage includes a base display/button firmware, Tera colors/tree, NVS settings loading and project planning. Chat, audio recording/playback, keyboard support and the installer are not implemented yet. No physical hardware validation has been performed. Build with `pio run` using PlatformIO.
