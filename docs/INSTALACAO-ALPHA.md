# ESP32 Tera — Alpha 1.0 / Installation

[Site / Website](https://fwestarb.github.io/esp32-tera/) · [Download e SHA-256 / Release files](https://github.com/fwestarb/esp32-tera/releases/tag/v1.0.0-alpha.1)

## Português

Compatível com **TTGO T-Display original (ESP32, 4 MB)**. Não use este firmware em T-Display S3 ou placas diferentes. Esta alpha não inclui teclado nem alto-falante físico.

### Instalar no Windows

1. Instale **Python 3.12 de python.org**, com Python no PATH. Use um cabo USB de dados. O driver CP210x precisa estar instalado se a porta COM não aparecer.
2. Extraia o pacote ZIP em uma pasta sua. Execute `instalar.cmd`. A instalação cria ambientes Python dentro dessa pasta e baixa a voz feminina Dora (~338 MB de modelos, mais bibliotecas). Precisa de internet nesta etapa. Não há assinatura ou conta para a voz.
3. Encerre o aplicativo Tera se estiver aberto. Execute `gravar-firmware.cmd` e informe a porta da TTGO, por exemplo COM3. A gravação substitui o programa da placa e preserva a NVS. Não formate ou apague toda a flash.
4. Abra `configurar.cmd`. Se o Windows impedir o acesso, confira o antivírus, o cabo, o driver e se outro monitor serial ocupa a porta. Apenas um aplicativo pode usar a porta por vez.
5. Configure Wi-Fi 2,4 GHz e sua chave pessoal Groq. O GPT-OSS 120B é o padrão; 20B é uma alternativa. Senha/chave vazias mantêm os valores da mesma rede/provedor. A opção **Trocar somente o modelo** preserva todas as credenciais.

O pacote usa scripts Python; não é um executável autônomo. Os modelos de voz são instalados separadamente e não estão no ZIP. Depois de baixar, a síntese de voz funciona offline no PC. A conversa e a transcrição continuam exigindo internet.

### Conferir o download

Baixe também `SHA256SUMS.txt` na página da versão. No PowerShell, execute `Get-FileHash .\esp32-tera-1.0.0-alpha.1.zip -Algorithm SHA256` e compare o resultado com o arquivo. Esta verificação é opcional; o gravador verifica os hashes dos binários antes de usar a placa.

### Criar a chave gratuita

Abra https://console.groq.com/keys, crie sua chave pessoal e coloque-a no campo de configuração. Não compartilhe a chave. O projeto não ativa cobrança nem configura faturamento: use o plano gratuito e confira as cotas da sua conta. Nenhuma API oferece aqui uso gratuito ilimitado.

Groq oferece conversa e transcrição. OpenRouter é uma alternativa **somente de texto nesta alpha**: use sua chave de https://openrouter.ai/settings/keys, selecione `openrouter/free` ou um identificador terminado em `:free`. A disponibilidade do modelo e os limites dependem do serviço. Trocar o modelo de conversa não troca o transcritor do microfone.

### Configurar pelo celular

Segure GPIO0 por cerca de dois segundos com a placa ligada. Conecte à rede TERA-… usando a senha mostrada na tela. Mantenha a conexão mesmo que o celular informe ausência de internet. Abra http://192.168.4.1 e escolha Wi-Fi, provedor, modelo e chave. Salve; a rede temporária fecha. Também fecha após dez minutos. Não segure GPIO0 durante reset, pois ele seleciona o bootloader.

### Usar

Segure GPIO35, fale e solte para enviar (0,5 a 10 segundos). A placa mostra a resposta; GPIO0 avança as páginas. No PC, deixe o aplicativo ativo para falar as respostas e abrir os programas permitidos. O X minimiza; **Encerrar assistente** fecha. **Parar voz** interrompe a reprodução.

Pedidos aceitos incluem “pode abrir a calculadora?”, “Tera, abre o Bloco de notas, por favor” e “quero abrir o YouTube”. Alvos: calculadora, Bloco de notas, explorador de arquivos, navegador, GitHub e YouTube. Um aplicativo por pedido. Pedidos incertos pedem esclarecimento. Existem variantes e alguns aliases; não há tolerância universal a qualquer erro de fala.

Para iniciar com o Windows, execute `powershell -NoProfile -File tools/install_pc_startup.ps1`. Para remover, apague o atalho ESP32 Tera da pasta Inicializar do usuário. Nenhum serviço é instalado.

### Erros e limites

- **401:** chave inválida ou sem acesso ao provedor.
- **429:** cota ou capacidade do serviço; aguarde ou escolha outro modelo compatível.
- **Modelo recusado:** selecione outro modelo; não há troca silenciosa para um modelo pago.
- **Wi-Fi:** verifique nome exato, senha, sinal e rede 2,4 GHz.
- **USB:** encerre outros aplicativos seriais; reconecte o cabo se a comunicação física parar.
- **Voz:** confira os alto-falantes e execute novamente a instalação da Dora. Faber e vozes Windows são alternativas explícitas.

Chaves ficam na NVS sem criptografia. Não publique dumps da flash. Áudio vai para Groq; perguntas e uma troca anterior vão para o provedor de chat. A voz de saída é sintetizada localmente em WAV temporário e removida após reprodução. O histórico fica apenas na RAM e pode ser limpo. Requisições HTTPS são bloqueantes e têm timeout; não há streaming nesta alpha.

## English

Supports the **original TTGO T-Display, ESP32 with 4 MB flash**, not T-Display S3. Install Python 3.12 with PATH enabled on Windows, extract the ZIP and run `instalar.cmd`. This installs isolated Python environments and downloads offline Portuguese female Dora speech. Internet is needed for installation. Voice models are not included in the ZIP; this is a Python application, not a standalone executable.

Close serial applications, run `gravar-firmware.cmd`, and enter the board COM port. Firmware flashing preserves NVS. Open `configurar.cmd` to configure 2.4 GHz Wi-Fi, your own Groq key and chat model. GPT-OSS 120B is the default, with 20B available. Model-only updates preserve credentials. Groq supports speech input; OpenRouter currently supports text only and requires `openrouter/free` or a `:free` model. Free accounts have quotas and availability limits; the project does not enable billing.

For phone setup, hold GPIO0 for about two seconds while running, connect to the password-protected TERA network, then visit http://192.168.4.1. The password appears on the board display. The setup network has no internet and closes after saving or ten minutes. Do not hold GPIO0 during reset.

Hold GPIO35 to speak and release to send, up to ten seconds. GPIO0 advances response pages. PC audio and allowlisted desktop actions require the Windows companion to remain active. Closing the window minimizes it; the explicit Quit button stops it. Supported Portuguese opening requests cover Calculator, Notepad, Explorer, browser, GitHub and YouTube. Ambiguous targets ask for clarification; arbitrary shell commands are never executed.

API credentials remain in unencrypted NVS. Do not distribute flash dumps. Audio/text are sent to the respective cloud providers. Speech output is generated locally and uses a temporary WAV. Memory contains only one bounded previous exchange. Physical speaker and keyboard are not implemented. See the test report for verified behavior and remaining hardware limitations.

To verify the download, compare `Get-FileHash .\esp32-tera-1.0.0-alpha.1.zip -Algorithm SHA256` with `SHA256SUMS.txt` from the release. The firmware installer also verifies each binary against its manifest before flashing.
