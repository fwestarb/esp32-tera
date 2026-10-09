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
4. GPIO0 refaz a calibracao; nao segure esse botao durante reset.

Media e a polarizacao DC (escala ADC de 0 a 4095); RMS mede variacao sem a componente DC; pico a pico e maximo menos minimo. Clipping conta amostras perto dos extremos. O teste mostra atividade eletrica, nao reconhece voz nem confirma qualidade de audio. Uma entrada solta tambem pode produzir variacao. O limiar e indicativo, nao uma medida em decibeis.

Se o sinal ficar no limite, revise alimentacao, polarizacao e ganho. Se nao acompanhar sua fala, confira circuito e conexoes. Nao conclua que o microfone funciona apenas porque a barra mexeu.

Para restaurar o chat:
```text
pio run -e ttgo_t_display -t upload --upload-port COM3
```
