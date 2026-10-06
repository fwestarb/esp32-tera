# ESP32 Tera

Chat de IA em desenvolvimento para a **TTGO T-Display original**. Visual minimalista em cinza e verde, com uma árvore que recebe folhas após uma resposta real.

## Estado mais recente — 0.2.2

Wi-Fi e HTTPS foram confirmados na TTGO com as credenciais do usuário. Corrigida falha de validação da cadeia TLS: agora são usadas raízes Google Trust Services em PEM, com validação ativa. Modelo atual fixado em `google/gemma-4-26b-a4b-it:free`; a consulta retornou **HTTP 429**, portanto uma resposta útil desse modelo ainda não foi validada. Não há fallback pago. Aguarde disponibilidade/limite do serviço antes de repetir. Áudio e teclado continuam pendentes. Consulte `docs/RESULTADOS.md` e `docs/TESTE-IA-REAL.json`.

## Base implementada — firmware 0.2.0 (histórico)

Implementado: configuração de Wi-Fi/chave pessoal pela USB, gravação em NVS, conexão Wi-Fi, HTTPS com certificados verificados, requisição de texto para OpenRouter, respostas na tela com páginas pelo botão GPIO0 e interface local de configuração/chat.

**Verificado na placa física:** gravação com hash conferido pelo esptool, inicialização, comunicação USB e 11 verificações de protocolo/validação. Compilação: RAM estática 46.460 bytes (14,2%); flash 1.012.849 bytes (77,3% da partição). RAM livre medida antes de conectar Wi-Fi: 245.992 bytes; não representa o pico de TLS.

**Pendente de validação:** conexão à rede do usuário e uma resposta real da API. Sem esses testes, a integração não deve ser apresentada como validada. Aparência física da tela e botões requerem confirmação visual. Áudio, teclado e operação sem computador ainda não estão concluídos. Nesta etapa, as mensagens são digitadas no computador; o pedido à IA é realizado pela ESP32.

## Usar agora

1. Conecte a placa por USB e feche outros monitores seriais.
2. Abra `configurar.cmd` (Python e pyserial; já disponíveis neste computador).
3. Selecione a porta da placa, clique **Conectar**.
4. Digite o nome e senha da rede **2,4 GHz** e uma chave pessoal OpenRouter. Clique **Salvar configuração na placa**.
5. Aguarde Wi-Fi conectado e a sincronização de horário. Digite uma pergunta e clique **Enviar à IA**.
6. Leia a resposta na interface e na TTGO. O botão GPIO0 avança as páginas. Não segure GPIO0 durante o reset.

As perguntas são independentes; ainda não há histórico conversacional. O modelo é fixado em `openrouter/free`; disponibilidade e limites dependem do provedor. Uma consulta pode bloquear os botões/USB enquanto aguarda a resposta (timeout de leitura 45 s). Não é streaming. O display usa transliteração de acentos; a USB mantém UTF-8.

A chave não é inserida no código e a ferramenta não grava credenciais em arquivo no computador. Ela é armazenada na NVS da placa **sem criptografia**. Quem puder ler a flash poderá recuperá-la. Backups privados da placa não devem ser publicados.

## Compilar e gravar

```text
python -m pip install -r tools/requirements.txt
pio run
pio run -t upload --upload-port COM3
python tools/test_device.py
```

O teste usa COM3 por padrão, não grava configuração válida e pode ser adaptado para outra porta. O firmware não importa `config.example.json`: esse arquivo descreve opções futuras.

## Evidências e planejamento

- [Resultado dos testes físicos USB](docs/TESTES-USB.json)
- [Relatório da etapa](docs/RESULTADOS.md)
- [Planejamento](docs/PLANO.md)
- [Hardware](docs/HARDWARE.md)
- [Provedores](docs/PROVEDORES.md)
- [OpenRouter: API oficial](https://openrouter.ai/docs/quickstart)

## English

AI chat firmware for the original TTGO T-Display. Version 0.2.0 implements local USB configuration, persistent NVS credentials, Wi-Fi, certificate-verified HTTPS and text requests to OpenRouter. A Python desktop tool provides configuration and text input; the ESP32 makes the API request and displays the reply. GPIO0 advances pages.

The firmware was built, flashed with esptool hash verification and exercised on the physical board through 11 USB/protocol checks. User Wi-Fi association and a real API response are **not yet verified**. Audio, keyboard input and standalone operation remain pending. Each question is independent. Requests block the main loop while waiting; this is not streaming.

Run `configurar.cmd`, connect to the board, enter your 2.4 GHz Wi-Fi credentials and personal OpenRouter API key, then save and send a question. The key is stored in unencrypted NVS, never embedded in source code. Do not publish flash backups.


### Atualização 0.2.1: diagnóstico Wi-Fi

A interface bloqueia o envio até a conexão e mostra o motivo de desconexão. Clique **Diagnosticar Wi-Fi** para verificar se a rede configurada aparece em 2,4 GHz; a busca interrompe a conexão temporariamente e depois tenta novamente. O firmware 0.2.1 compilou com 46.484 bytes de RAM estática e 1.015.465 bytes de flash. Os números e testes anteriores acima pertencem à versão 0.2.0.
