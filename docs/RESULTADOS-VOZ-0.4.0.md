# Validacao de voz 0.4.0 — 09/10/2026

## Resultado real

A **propria TTGO** enviou uma gravacao anterior de dez segundos para transcricao na Groq e consultou a IA com o texto recebido. A chave permaneceu na NVS da placa. O PC forneceu a gravacao de teste por USB; **esse replay nao e uma nova captacao nem um teste do botao**.

| Etapa | Resultado |
| --- | --- |
| Autenticacao da chave Groq | HTTP 200 |
| Envio de audio pela ESP32 | HTTP 200; 80.524 bytes de multipart lidos pelo cliente |
| Transcricao | Este é um áudio de 10 segundos para verificar se consegue entender todas as palavras. 1, 2, 3... |
| Resposta real da IA | Sim, consegui entender todas as palavras. |
| Protocolo USB da versao final | 10 verificacoes passaram |
| Estado apos consulta | Wi-Fi conectado, voz pronta; 234812 bytes livres |

Evidencia completa: [TESTE-VOZ-REPLAY-0.4.0.json](TESTE-VOZ-REPLAY-0.4.0.json). Contem o hash da gravacao original; o arquivo de audio permanece fora do Git.

## Firmware instalado

- TTGO T-Display original, GPIO32 analogico e botao GPIO35 para falar.
- Segurar para gravar, soltar para enviar; limite de dez segundos.
- Build: 46.672 bytes de RAM estatica, 985.845 bytes de flash. Pilha principal 16 KB, alocada dinamicamente; nao aparece no valor de RAM estatica.
- Upload concluido com hash verificado pelo esptool; configuracao Wi-Fi/Groq preservada.
- SHA-256 do binario instalado: `44510354bd4ca8671de4dc5b4c7a14882c9b8bf027b83d6ef4cbdb0241670bf3`.
- Certificados HTTPS verificados; nao foi usado `setInsecure`.

## Problemas encontrados e corrigidos

1. Dois blocos de 80 KB nao puderam ser alocados devido a fragmentacao da RAM.
2. Dividir em blocos de 16 KB permitiu captar, mas o TLS falhou por falta de um bloco adequado. A versao final usa um unico buffer contiguo de 80 KB em PCM8 centrado no nivel DC do ADC.
3. O log de panic e o backtrace apontaram para a verificacao de assinatura ECDSA do certificado dentro da cadeia de chamadas da transcricao. A pilha padrao de 8 KB era insuficiente e foi aumentada para 16 KB. A versao final concluiu transcricao e chat, sem reiniciar. O minimo de pilha livre informado pelo firmware apos a consulta foi `6716`.
4. O envio agora cede tempo ao sistema entre blocos. O monitor USB abre sem alternar DTR/RTS e evita resetar a placa ao observar logs.

A primeira captura integrada ocorreu antes dessas correcoes: 80.000 amostras em 9.999.965 us, RMS 9,05, nenhum corte no ADC; falhou no HTTPS. A captacao e transcricao local dos testes anteriores ja tinham sido validadas. A integracao atual foi verificada usando essa gravacao anterior conhecida, e nao simulacao de IA.

## Falta confirmar fisicamente

- Nova fala segurando e soltando **GPIO35**, com o formato de captura atual.
- Paginacao pelo GPIO0 e aparencia/legibilidade do resultado na tela.
- Uso com o PC desligado: a arquitetura nao precisa dele, mas o teste ainda requer alimentar a placa por outra fonte USB adequada.
- Teclado e alto-falante permanecem sem implementacao; esta versao responde por texto.

As perguntas sao independentes e o reconhecimento nao e infalivel. Ruido/voz fraca podem afetar o resultado; veja [VOZ.md](VOZ.md). Audio sai para a Groq; use uma chave de conta no plano Free, sujeito a cotas. Nao publique segredos, backups da flash ou falas privadas.

## English

The physical TTGO uploaded a previously captured ten-second clip, received a correct Groq transcription and a real AI reply. This replay validates cloud integration, not a fresh live recording or the push-to-talk button. Ten USB checks passed on the final firmware. Audio heap fragmentation and TLS stack overflow were fixed while keeping certificate validation enabled. Fresh button-driven input, display/paging confirmation and unplugged-PC operation remain to be checked by the user.
