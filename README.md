# ESP32 Tera

Chat de IA para a **TTGO T-Display original**, com identidade minimalista em cinza e verde. A arvore recebe folhas quando uma resposta real chega.

## Versao 0.4.0: entrada de voz

Implementado: microfone analogico em GPIO32, segurar GPIO35 para falar e soltar para enviar, limite de dez segundos, transcricao em portugues pela Groq e consulta de IA com resposta na tela. GPIO0 avanca as paginas. Audio e enviado diretamente pela placa; o computador serve para instalar/configurar ou observar logs.

**Verificado na placa fisica:** build/upload com hash conferido, Wi-Fi, dez testes USB e HTTPS com audio em memoria. A ESP32 enviou uma gravacao anterior de dez segundos para a Groq, recebeu a transcricao correta e uma resposta real da IA. Erros de memoria e pilha encontrados no desenvolvimento foram corrigidos. **Ainda falta confirmar uma nova fala acionada pelo botao GPIO35 e o uso sem PC**. Veja [resultados](docs/RESULTADOS-VOZ-0.4.0.md) e [guia de voz](docs/VOZ.md).

## Usar

1. Instale com `pio run -e ttgo_t_display -t upload --upload-port COM3`.
2. Abra `configurar.cmd`, conecte a placa e selecione **groq**. Informe Wi-Fi 2,4 GHz e sua chave pessoal Groq, usando uma conta no plano Free. Salve na placa. Se ja estiver configurada, mantenha as credenciais.
3. Aguarde Wi-Fi e sincronizacao do relogio. Segure GPIO35, fale perto do microfone e solte.
4. Leia a resposta na tela; GPIO0 muda a pagina. Nao segure GPIO0 durante o reset.

A interface USB ainda aceita perguntas digitadas. OpenRouter permanece opcao de texto; voz requer Groq nesta versao. Audio nao e reproduzido por alto-falante. Teclado e historico conversacional permanecem pendentes. Cada pergunta e independente; durante a consulta os botoes ficam bloqueados.

## Dados e gratuidade

A chave fica na NVS **sem criptografia**, fora do codigo. Nao publique backups da flash. Audio temporario fica na RAM e e enviado ao provedor para transcricao; os logs opcionais mostram a fala reconhecida e a resposta. Gravacoes locais de desenvolvimento ficam fora do Git.

Groq Free tem limites de texto e audio; nao e ilimitado. O firmware nao verifica faturamento e nao ativa planos. Cada pergunta falada usa um pedido de transcricao e outro de chat. Sem Wi-Fi ou credenciais validas, nao ha IA funcional/offline.

## Desenvolvimento e evidencias

```text
python -m pip install -r tools/requirements.txt
pio run -e ttgo_t_display
python tools/test_device.py
python tools/voice_check.py --op check
```

- [Voz: uso, memoria, amostragem e limites](docs/VOZ.md)
- [Verificacoes USB da versao 0.4.0](docs/TESTES-USB-0.4.0.json)
- [Memoria e multipart na placa](docs/TESTE-AUDIO-MEMORIA-0.4.0.json)
- [Transcricao e resposta reais pela placa, com gravacao anterior](docs/TESTE-VOZ-REPLAY-0.4.0.json)
- [Resultados e limites da verificacao](docs/RESULTADOS-VOZ-0.4.0.md)
- [Teste independente de microfone/gravador USB](docs/TESTE-MICROFONE.md)
- [Configuracao Groq](docs/GROQ.md)
- [Planejamento](docs/PLANO.md)
- [Hardware](docs/HARDWARE.md)
- [Historico de versoes](docs/HISTORICO-README-0.3.md)

## English

AI chat firmware for the original TTGO T-Display. Hold GPIO35 to record Portuguese speech and release to send; the analog amplifier connects to GPIO32. Groq Whisper transcribes the recording, GPT OSS answers on the display, and GPIO0 advances pages. Maximum ten seconds; audio output, keyboard and conversation history are not implemented.

Version 0.4.0 was built and flashed with hash verification. Wi-Fi, ten USB checks and HTTPS with an allocated audio buffer passed. The physical ESP32 uploaded a previously recorded ten-second test clip, received the correct transcription and a real AI reply. Audio/TLS heap fragmentation and TLS stack overflow were corrected. A fresh GPIO35-triggered recording and operation with the PC disconnected still require user confirmation. See the voice guide and test reports.

Configure your 2.4 GHz Wi-Fi and personal Groq Free key through `configurar.cmd`. Runtime requests are made by the ESP32; only power and Wi-Fi are needed after setup. Audio leaves the device for cloud transcription; API keys remain in unencrypted NVS. Quotas apply and billing status is not checked by firmware. Do not publish flash backups or private voice logs.
