# ESP32 Tera

Chat de IA para a **TTGO T-Display original**, com tela minimalista em cinza e verde e uma árvore que ganha folhas quando chega uma resposta real.

## Versão 0.5.0

- **Voz de entrada:** microfone analógico em GPIO32. Segure GPIO35 para falar e solte para enviar; até dez segundos. A placa faz a transcrição pela Groq e mostra a resposta na tela.
- **Voz no computador:** o aplicativo USB fala as respostas com a voz em português instalada no Windows. Não requer uma segunda API. Pode ser desativada ou interrompida; para ao iniciar uma gravação.
- **Comandos no PC:** frases definidas abrem calculadora, Bloco de notas, explorador, navegador, GitHub ou YouTube. A IA não executa código nem escolhe comandos arbitrários.
- **Configuração pelo celular:** rede temporária com senha na tela e página local para Wi-Fi e chave pessoal de IA. Fecha após salvar ou depois de dez minutos.
- **Contexto curto:** a última troca fica na RAM, com limites de tamanho e opção de limpar. Não é um histórico permanente.

**Verificado em 10/10/2026:** firmware compilado e instalado com hash conferido; dez testes USB, nove testes HTTP/DNS na placa, sete verificações da página móvel e dezessete testes do aplicativo PC. A placa transcreveu uma gravação anterior e a resposta foi reproduzida pelo Windows. Um comando de voz sintetizado abriu a calculadora. Veja [evidências e limites](docs/RESULTADOS-0.5.0.md).

## Usar

### No PC

1. Instale Python 3.12 e as dependências: `python -m pip install -r tools/requirements.txt`.
2. Abra `configurar.cmd`. O aplicativo identifica a TTGO por USB e conecta automaticamente. Deixe-o aberto, mesmo minimizado, para voz e comandos no PC.
3. Com Wi-Fi e relógio sincronizados, segure GPIO35, fale e solte. Também é possível digitar perguntas e comandos na interface.
4. Exemplos: **“abra a calculadora”**, **“abrir GitHub”**, **“abra o Bloco de notas”**. Frases diferentes seguem para o chat; não há controle livre do computador.

Para iniciar o aplicativo ao entrar no Windows: `powershell -NoProfile -File tools/install_pc_startup.ps1`. A instalação só cria um atalho na pasta Inicializar do seu usuário. Para remover, apague o atalho **ESP32 Tera** nessa pasta. Fechar o aplicativo encerra a integração com o PC; minimizar mantém o funcionamento.

### Pelo celular

1. Segure **GPIO0 por cerca de dois segundos** enquanto a placa está ligada. Não segure esse botão durante o reset, pois ele também seleciona o modo de gravação do ESP32.
2. Conecte o celular à rede **TERA-…**, usando a senha mostrada na tela. Ela é temporária e não oferece internet; mantenha a conexão mesmo se o celular avisar disso.
3. Abra **http://192.168.4.1** no navegador. Informe Wi-Fi 2,4 GHz, provedor e chave pessoal. Campos secretos vazios mantêm as credenciais do mesmo provedor/rede; para outra rede informe a senha. Marque a opção correspondente se sua rede não tiver senha.
4. Salve. A rede Tera fecha e a placa tenta conectar ao seu Wi-Fi. Se precisar reconfigurar, abra a rede novamente pelo botão.

Sem configuração, o portal abre ao iniciar. Após uma falha prolongada de conexão, também abre uma vez. Enquanto a configuração está aberta, consultas de IA ficam pausadas.

![Página de configuração móvel](docs/images/configuracao-celular.png)

*Captura da página real em navegador com largura de celular; dados de exemplo e servidor simulado. A validação HTTP/DNS foi feita separadamente na ESP32.*

## Instalar o firmware

```text
pio run -e ttgo_t_display -t upload --upload-port COM3
```

A instalação preserva a configuração NVS existente. Groq é usado para texto e voz; OpenRouter permanece opção de texto. GPIO0 também avança páginas com um toque curto.

## Dados, gratuidade e limites

A chave fica na NVS **sem criptografia**, fora do código; não publique backups da flash. O aplicativo não salva chaves ou gravações no PC. A interface pode mostrar transcrições e respostas. Áudio temporário é enviado ao provedor para transcrição; perguntas e uma troca anterior vão para o chat.

Use sua chave pessoal de uma conta gratuita. As APIs têm cotas e podem responder 429 mesmo na primeira tentativa, por capacidade ou limite. O firmware não verifica faturamento nem ativa planos. Não há IA local/offline.

A montagem não tem alto-falante nem teclado. A voz de saída depende do **Windows com o aplicativo ativo e uma saída de áudio**. A placa continua usando a tela quando não há PC. Durante as consultas HTTPS, não é possível interromper a requisição pelos botões; há timeouts, sem streaming. A captura continua limitada a dez segundos e amostragem por software.

## Desenvolvimento

```text
python tools/test_pc_bridge.py
python tools/test_device.py
python tools/test_pc_device.py
```

O primeiro teste não usa hardware. Os demais usam a placa; o teste integrado usa arquivos locais de áudio de desenvolvimento, que não são publicados no Git, e consome chamadas da sua API. A página pode ser testada com Node.js, Playwright e Edge: `node tools/test_portal_ui.cjs`.

- [Resultados 0.5.0](docs/RESULTADOS-0.5.0.md)
- [Voz e limites de memória/amostragem](docs/VOZ.md)
- [Configuração Groq](docs/GROQ.md)
- [Hardware](docs/HARDWARE.md)
- [Planejamento](docs/PLANO.md)
- [Evidências anteriores 0.4.0](docs/RESULTADOS-VOZ-0.4.0.md)

## English

AI chat for the original TTGO T-Display. Hold GPIO35 to record Portuguese speech and release to submit (maximum ten seconds). The ESP32 sends audio to Groq for transcription, then displays the AI response. Short-press GPIO0 to advance pages; hold it for about two seconds to open a temporary password-protected configuration Wi-Fi network. Connect a phone using the password on the display and browse to **http://192.168.4.1**. The portal closes after saving or ten minutes. Never hold GPIO0 during reset.

The Windows USB companion automatically connects and reads replies using the installed Portuguese voice, without another API. It can open a fixed set of applications/sites through explicit Portuguese commands: calculator, Notepad, Explorer, browser, GitHub and YouTube. AI responses are never interpreted as executable commands. Keep the companion running; minimizing is supported. Optional per-user startup installation creates a removable shortcut.

Version 0.5.0 was built/flashed with hash verification. Automated tests covered USB validation, HTTP/DNS on the physical ESP32, the mobile HTML page, Windows playback, a prerecorded microphone clip and a synthesized voice command that opened Calculator. These tests do **not** prove a fresh live microphone/button capture or connection from an actual phone. See the results report for exact scope.

API credentials stay in unencrypted NVS; cloud services receive audio/text and enforce quotas. One bounded previous exchange stays in RAM. No physical speaker or keyboard is included. PC speech/actions require Windows and the companion; standalone screen chat still needs Wi-Fi and power. HTTPS requests are blocking with timeouts, not streaming.
