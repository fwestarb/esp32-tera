# Teste analogico de microfone na GPIO32

## Resultado inicial em 09/10/2026

Build e upload para a TTGO em COM3 concluidos, com hash da gravacao verificado. Foram recebidas 18 janelas de leitura ADC pela USB. Na ultima: media 1934,7, pico a pico 32, RMS 4,73, ruido de referencia 4,96 e nenhuma amostra nos limites do ADC. Isso confirma que o programa executa e le a GPIO32; a correspondencia com fala ainda precisa do teste do usuario. Dados em TESTE-MIC-LEITURAS.json.

Teste independente do chat, sem Wi-Fi/API. Use a saida analogica do pre-amplificador em GPIO32 e GND comum. A GPIO32 nao pode receber tensao negativa nem maior que 3,3 V. Uma capsula electret sozinha precisa de polarizacao e pre-amplificacao; este programa nao substitui esse circuito.

Compilar:
```text
pio run -e mic_test
```

Instalar o teste na TTGO (substitui temporariamente o programa executado, preservando a configuracao NVS):
```text
pio run -e mic_test -t upload --upload-port COM3
pio device monitor -e mic_test --port COM3
```
Feche a interface Tera e outros monitores antes de abrir COM3.

1. Apos iniciar, fique em silencio durante a calibracao (cerca de 3 segundos).
2. Fale perto do microfone e compare a barra, RMS e pico a pico com o silencio.
3. Repita algumas vezes: os valores devem subir acompanhando a fala e baixar quando parar.
4. Na versao com gravador, GPIO0 inicia uma gravacao de cinco segundos; nao segure esse botao durante reset. A calibracao de ruido acontece automaticamente ao iniciar o programa.

Media e a polarizacao DC (escala ADC de 0 a 4095); RMS mede variacao sem a componente DC; pico a pico e maximo menos minimo. Clipping conta amostras perto dos extremos. O teste mostra atividade eletrica, nao reconhece voz nem confirma qualidade de audio. Uma entrada solta tambem pode produzir variacao. O limiar e indicativo, nao uma medida em decibeis.

Se o sinal ficar no limite, revise alimentacao, polarizacao e ganho. Se nao acompanhar sua fala, confira circuito e conexoes. Nao conclua que o microfone funciona apenas porque a barra mexeu.

Para restaurar o chat:
```text
pio run -e ttgo_t_display -t upload --upload-port COM3
```

## Gravar e ouvir

### Transcricao local experimental

Ferramenta `tools/transcribe_local.py` com dependencias em `tools/requirements-transcription.txt`, instalada em `.venv-transcription`. Os modelos sao baixados, mas o audio nao e enviado para um servico. Execute com o Python desse ambiente, passando o WAV e opcionalmente `--model small`.

No teste 2, os modelos base e small concordaram no inicio: "Teste falando mais longe do microfone". As hipoteses para o final divergiram e nao devem ser consideradas uma transcricao confiavel. Esse teste ocorreu no computador; nao e reconhecimento de fala executado pela ESP32 nem integracao de voz com a API.

Execute `python tools/record_microphone.py --port COM3 --output recordings/meu-teste`. O computador espera o firmware responder, envia o comando r e recebe 40.000 amostras ADC de 16 bits. A TTGO conta tres segundos e mostra GRAVANDO por cinco segundos; fale nesse intervalo. A transferencia acontece depois da captura, sem reduzir a velocidade de amostragem por causa da USB.

O script confere comprimento e CRC32, salva o ADC original, um WAV com a componente DC removida e uma copia `-escutar.wav` com ganho digital explicitamente registrado. O WAV e mono, PCM de 16 bits; a taxa usa a duracao medida na placa. Amplificacao digital tambem aumenta o ruido. Nao ha filtro de ruido, reconhecimento de fala ou correcao de jitter. Os arquivos de voz ficam em recordings/, ignorada pelo Git.

Captura de 09/10/2026: 40.000 amostras em 4.999.964 microssegundos, WAV 8.000 Hz, CRC confirmado, nenhuma amostra no limite ADC. RMS 10,56 e pico a pico 178 unidades ADC. 178 amostras iniciaram mais de 30 microssegundos depois do horario previsto (0,445%). Copia para ouvir com ganho digital de aproximadamente 18,95 vezes. A inteligibilidade aguarda audição do usuario; a gravacao nao foi apresentada como fala reconhecida.
