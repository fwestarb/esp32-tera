# Resultados — ESP32 Tera Alpha 1.0

Data: **11/10/2026**. Versão: `1.0.0-alpha.1`. Desenvolvimento e pacote locais; nada publicado e nenhum site criado.

## Implementado e verificado

| Área | Evidência |
|---|---|
| Firmware compilado e gravado | RAM estática 47.704 bytes; código flash 1.054.197 bytes, cerca de 80,4% da partição; gravação verificada pelo esptool. [Build](BUILD-ALPHA-1.0.json) |
| Protocolo USB físico | Dez verificações de status, entrada inválida, limites e preservação da configuração. [USB](TESTES-USB-1.0.0-alpha.1.json) |
| Pedidos de abertura | 21 frases executadas no classificador real da ESP32: pedidos educados, aliases, negativos, citações, alvos ambíguos e tentativas de comandos fora da lista. [Integração alpha](TESTES-ALPHA-1.0.json) |
| IA real e contexto | Consultas autenticadas pela placa aos modelos GPT-OSS 20B e 120B, com respostas sobre Tera e seu dispositivo. Troca do modelo sem recuperar as chaves no PC. Ambos responderam e tiveram reprodução feminina local. O 120B ficou selecionado. |
| Ação real no Windows | “Pode abrir a calculadora, por favor?” abriu a calculadora, recebeu confirmação do aplicativo e produziu fala com Dora. Pedido ambíguo pediu esclarecimento. |
| Portal físico | Nove casos HTTP/DNS, incluindo token ausente, configuração inválida, tamanho excessivo, segredos ausentes da página e salvamento que preserva credenciais. A placa voltou ao Wi-Fi e ficou pronta para voz após fechar o portal. |
| Portal móvel | HTML real em Edge headless com largura de 390 px, servidor simulado, troca de provedor/modelo e formulário. [Interface móvel](TESTE-INTERFACE-PORTAL-1.0.0-alpha.1.json) |
| Interface Windows | Seis verificações na interface Tk real: espaço dos controles, campos secretos, área de resposta, modelos por provedor, preservação da escolha durante atualização de estado e seleção da voz. [Interface PC](TESTE-INTERFACE-PC-ALPHA.json) |
| Proteções do aplicativo | 21 testes de regressão: sessão, deduplicação, alvos permitidos, ausência de execução de respostas da IA, USB interrompido, mensagens fragmentadas e controle da fala. Execute `python tools/test_pc_bridge.py`. |
| Reinício e voz de entrada | Calculadora e Dora antes/depois de reset por RTS; modelo 120B preservado; áudio sintetizado enviado à placa, transcrição Groq HTTP 200, comando executado e reprodução concluída. Conexão acompanhada por pelo menos 16 segundos em repouso. [Recuperação](TESTE-RECUPERACAO-1.0.0-alpha.1.json) |
| Pacote extraído | Dependências instaladas em outra pasta, modelos validados, firmware gravado pelo instalador do ZIP com hash verificado, modelo 120B preservado, aplicativo USB e Dora reproduzidos nessa pasta. [Pacote](TESTE-PACOTE-ALPHA.json) |
| Aplicativo ativo | Reconexão automática e segunda abertura reutilizando a mesma instância; voz Dora e firmware Alpha confirmados. [Windows](TESTE-APP-WINDOWS-1.0.0-alpha.1.json) |
| Voz feminina | Dora/Kokoro CPU, português brasileiro. Exemplo gerado: 24 kHz, 8,736 segundos, RMS 1747; síntese levou aproximadamente 5,1 segundos neste computador. [Voz](TESTE-VOZ-DORA-ALPHA.json) |

As consultas simples dos dois modelos ficaram na ordem de 10–15 segundos incluindo geração e reprodução da fala. São medições locais de poucas consultas, não um benchmark nem garantia de tempo. A naturalidade é subjetiva: o arquivo de exemplo permite ouvi-la. O teste confirma geração de áudio e conclusão da reprodução no Windows, não uma medição acústica dos alto-falantes.

## Recuperação e arquivos

A versão anterior tem histórico Git completo em bundle verificado e cópia da partição de aplicação de 1.310.720 bytes. Ficam em `private-backups/before-alpha`, fora do Git e do pacote. A NVS não foi exportada nem apagada. O modelo foi salvo na NVS e confirmado após reinício.

O ZIP local contém firmware, hashes, código e scripts de instalação. Modelos de voz, ambientes Python, gravações, credenciais e backups ficam fora do ZIP. Requer Python 3.12 e internet para instalar dependências/modelos. Não é um executável autônomo.

## Limites desta validação

- Não foi solicitada nova gravação de fala ao usuário. O microfone real já tinha sido validado anteriormente; nesta etapa a cadeia de transcrição/comando foi verificada com áudio sintetizado reproduzido pelo protocolo USB.
- Nenhum celular físico foi conectado ao portal nesta etapa. HTTP/DNS foram verificados na placa; apresentação e JavaScript foram verificados no navegador.
- Testes no mesmo computador Windows; não representam todos os PCs, drivers ou roteadores.
- A conta usada aceitou os modelos nos testes. Isso não comprova cota ilimitada nem ausência de cobrança em uma conta que tenha faturamento ativado. O projeto não altera planos ou faturamento.
- Aberturas usam padrões locais e uma lista de alvos; não existe interpretação universal de qualquer frase nem execução de comandos gerados pela IA.
- Sem teclado físico ou alto-falante na placa. OpenRouter permanece somente texto. Sem streaming e com memória de uma troca anterior. Watchdog configurado, mas o timeout não foi provocado intencionalmente nesta etapa.

## English

Local `1.0.0-alpha.1` was compiled/flashed with integrity verification on the original TTGO T-Display. Physical tests covered USB validation, 21 intent phrases, real authenticated GPT-OSS 20B/120B chat, actual Windows Calculator launch with acknowledgement, female Dora playback, and HTTP/DNS portal behavior with preserved credentials. Reset recovery retained the model and restored the PC session; synthetic voice transcription returned HTTP 200 and triggered the allowlisted action. The portal HTML and Tk interface were tested separately. Reports above record the scope.

No new human microphone recording, actual phone connection, acoustic speaker measurement or second Windows computer was tested. Free API plans have quotas. Offline speech runs on the PC; AI/transcription require cloud access. No website or public release was created. The ZIP requires Python 3.12 and separately installed voice models.
