# Voz na TTGO original — firmware 0.4.0

## Uso

1. Alimente a TTGO e aguarde o Wi-Fi e o relogio HTTPS.
2. O amplificador analogico permanece conectado em **GPIO32**, com terra comum e alimentacao 3,3 V.
3. Segure o botao **GPIO35**, fale perto do microfone e solte para enviar. O outro botao, GPIO0, muda a pagina da resposta. Nao segure GPIO0 durante o reset.
4. A tela mostra GRAVANDO, Transcrevendo e Consultando IA. O limite e 10 segundos; ao atingir esse limite, o envio acontece automaticamente e o botao precisa ser solto antes da proxima gravacao.
5. A resposta aparece na tela. Nao existe reproducao de voz nesta versao.

A placa envia o audio diretamente por HTTPS para a **Groq** (Whisper large v3 turbo, idioma portugues), recebe o texto e consulta `openai/gpt-oss-20b`. Cada pergunta e independente. Sao dois pedidos de API por pergunta falada. Nao executa comandos no computador nem controla aparelhos; permite fazer pedidos e perguntas a IA por voz.

O computador serve para instalar/configurar e observar logs. O envio de voz pelo botao e feito pela ESP32. Para testar sem PC, a placa ainda precisa de alimentacao USB ou outra fonte adequada e Wi-Fi. Nao ha processamento de IA local/offline.

## Configuracao

Abra `configurar.cmd`, selecione `groq` e salve Wi-Fi 2,4 GHz e sua chave `gsk_...`. Uma chave OpenRouter serve somente para texto nesta versao. Mantenha sua conta Groq no plano Free; existem cotas de audio e texto. O firmware nao verifica faturamento nem ativa planos. Nao ha provedor pago de reserva.

Nao envie chaves pelo chat. A chave fica na NVS **sem criptografia**, fora do codigo. O audio e temporario na RAM e nao e salvo na flash ou enviado por USB. Ele sai da placa para o provedor quando voce solta o botao. Logs USB mostram transcricao/resposta: evite publicar falas privadas.

## Captura e limites

- ADC1 GPIO32, 12 bits, atenuacao 11 dB, amostragem nominal 8 kHz. Wi-Fi continua ativo; interferencia de radio na montagem analogica ainda pode afetar o sinal.
- Ate 80.000 amostras em um unico buffer contiguo de 80 KB. O centro DC e medido antes de gravar; cada amostra ADC de 12 bits e centralizada e armazenada em **PCM de 8 bits**, preservando passos de uma contagem ADC dentro de aproximadamente +/-127 contagens. Sinais mais fortes podem saturar esse formato, mesmo sem saturar o ADC.
- Apos captar, remove a media residual e aplica ganho digital limitado. O envio WAV multipart e por pequenos blocos, sem criar outra copia do audio. O buffer e liberado antes de consultar o chat.
- A tentativa de guardar ADC em blocos de 16 KB fragmentou a RAM e falhou no TLS, apesar de haver memoria livre. A versao atual valida HTTPS com o buffer unico ocupado no autoteste.
- A pilha da tarefa principal foi aumentada de 8 KB para 16 KB: o log e o backtrace mostraram reinicializacao durante verificacao ECDSA do certificado na cadeia de chamadas da voz. A transferencia tambem cede tempo ao sistema entre blocos. Os certificados continuam sendo verificados.
- O ADC e lido com temporizacao por software, nao por DMA. Contagem, duracao real e atrasos sao registrados; a taxa do WAV e calculada a partir da duracao medida. Nao representa amostragem profissional sem jitter.
- Capturas abaixo de meio segundo, RMS abaixo de `max(6; ruido*1,45)` ou saturacao acima de 1% sao rejeitadas antes da API. O nivel inicial de ruido e 4,6 contagens, baseado no teste silencioso anterior.
- **Calibrar ruido** na interface USB mede um segundo: fique em silencio. Essa calibracao vale ate reiniciar. Ruido forte ainda pode passar pelo filtro, e fala fraca pode ser rejeitada.
- A resposta de transcricao precisa ter texto e pelo menos um segmento com `no_speech_prob < 0,6`. Isso reduz transcricoes de silencio; nao garante reconhecimento correto. Confira o que a IA entendeu.
- Durante HTTPS, botao/USB ficam bloqueados. Ha timeouts de conexao/leitura; nao e streaming.

## Verificar e restaurar o teste anterior

```text
pio run -e ttgo_t_display -t upload --upload-port COM3
python tools/test_device.py
python tools/voice_check.py --op check
python tools/voice_check.py --op monitor --seconds 60
```

`check` testa alocacao, conversao e tamanho do multipart com 80.000 amostras sinteticas; nao e um teste de reconhecimento de fala. `monitor` observa o botao real. `voice_test` faz contagem de tres segundos e capta dez segundos automaticamente, pelo mesmo fluxo de transcricao/chat. Use quando estiver pronto para falar.

`voice_fixture.py caminho-do-teste.wav` permite testar a integracao com uma gravacao anterior: envia PCM8 por USB com CRC verificado e a placa faz transcricao/chat HTTPS. O comando `voice_fixture` e um diagnostico USB, nao e apresentado como captacao ao vivo e nao comprova o botao. A chave nunca sai da placa para o computador. WAV de teste deve ser mono, 16 bits, 8 kHz e ter entre meio e dez segundos.

Para voltar ao gravador USB independente: `pio run -e mic_test -t upload --upload-port COM3`. A configuracao NVS e preservada.

## English

Hold GPIO35 to record, release to submit; maximum ten seconds. Analog input is GPIO32. Groq Whisper transcribes Portuguese speech and GPT OSS answers on the display; GPIO0 advances pages. No speaker output, keyboard, conversation history or device control is implemented. Wi-Fi and power are required; the PC is only needed for installation/configuration and optional logs.

Audio is normalized and sent as 8-bit mono WAV over certificate-verified HTTPS, with software sampling at nominal 8 kHz. Temporary audio stays in RAM until uploaded; API credentials remain in unencrypted NVS. Use your own Groq Free key, subject to quotas. Short/weak/clipped captures and high no-speech results are rejected, but recognition and silence filtering are imperfect.

Sources: [Groq speech API](https://console.groq.com/docs/speech-to-text), [limits](https://console.groq.com/docs/rate-limits).
