# Resultados — ESP32 Tera 0.5.0

Verificação automática em **10/10/2026**, na TTGO T-Display original conectada em COM3 e no Windows deste computador. Nenhuma etapa exigiu que o usuário gravasse uma fala nova ou executasse testes.

## Implementado e instalado

- Portal de configuração Wi-Fi com senha temporária na tela, DNS de redirecionamento e formulário para Wi-Fi/provedor/chave. A rede fecha após salvar ou dez minutos. GPIO0 abre/fecha com uma pressão longa; o toque curto muda a página.
- Aplicativo Windows com conexão USB automática, síntese de voz local em português e controles para desligar fala/ações. A fala para ao iniciar uma gravação; minimizar mantém o aplicativo ativo.
- Abertura de calculadora, Bloco de notas, explorador, navegador, GitHub e YouTube por frases definidas. Sessão USB, confirmação de resultado e proteção contra repetição de um mesmo evento. Respostas geradas pela IA não são comandos executáveis.
- Uma troca anterior de contexto em RAM, limitada a 512 bytes de pergunta e 1024 bytes de resposta, sem cortar caracteres UTF-8. Botão para limpar conversa.
- Validação de configuração compartilhada entre USB e portal, preservação opcional de campos secretos e gravação evitada quando nada mudou. Removida a pausa fixa entre transcrição e chat. Buffer de recepção USB maior e leitura PC tolerante a mensagens/UTF-8 fragmentados.

O firmware foi gravado com o hash da flash conferido pelo esptool. **RAM estática: 47.680 bytes; código flash: 1.046.597 bytes**, 79,8% da partição de aplicação. A configuração anterior foi preservada. [Build e hash do arquivo](BUILD-0.5.0.json).

## Evidências

| Verificação | Resultado | Evidência |
|---|---|---|
| Validação USB na placa | 10 verificações passaram; entradas inválidas não mudaram a configuração | [USB](TESTES-USB-0.5.0.json) |
| HTTP e DNS do portal na placa | 9 casos passaram, incluindo rejeição sem token, JSON inválido, corpo grande e salvamento válido preservando credenciais | [Portal físico](TESTE-PORTAL-0.5.0.json) |
| Formulário móvel | 7 verificações passaram no Edge sem interface, em largura de 390 px | [Página](TESTE-INTERFACE-PORTAL-0.5.0.json) |
| Código do aplicativo PC | 17 testes passaram: sessão, repetição, comandos desconhecidos, modo desativado, cancelamento, recuperação e UTF-8 dividido | [Testes PC](TESTES-PC-0.5.0.json) |
| Voz local Windows | WAV não silencioso, voz Microsoft Maria Desktop pt-BR | [Síntese](TESTE-FALA-WINDOWS-0.5.0.json) |
| Gravação anterior pela placa | Groq HTTP 200, transcrição coerente, resposta real e reprodução Windows concluída | [Integração](TESTE-INTEGRACAO-PC-0.5.0.json) |
| Comando por voz | Áudio sintetizado “Abra a calculadora.” transcrito na placa; calculadora aberta e confirmação falada | [Integração](TESTE-INTEGRACAO-PC-0.5.0.json) |
| Contexto curto | Após informar o nome Felipe, a pergunta seguinte recebeu “Felipe”; contexto limpo ao final | [Integração](TESTE-INTEGRACAO-PC-0.5.0.json) |
| Aplicativo em execução | Conexão automática COM3 e abertura de segunda cópia restaurando a mesma instância | [Aplicativo real](TESTE-APP-WINDOWS-0.5.0.json) |

No replay da gravação de dez segundos, a transcrição foi: “Este é um áudio de 10 segundos para verificar se consegue entender todas as palavras. 1, 2, 3...”. A resposta real foi **“Entendi o texto. Tudo certo!”**, reproduzida pela voz do Windows. O comando sintetizado recebeu **“Solicitei ao PC que abrisse a calculadora.”**, também reproduzido.

A integração terminou com Wi-Fi e voz prontos, **229.328 bytes de heap livre** e margem de pilha de **6.332 bytes**. O contexto foi limpo. O aplicativo ficou instalado para iniciar minimizado ao entrar no Windows, por um atalho **ESP32 Tera** na pasta Inicializar do usuário, e está conectado à placa. As opções de voz e comandos começam ativadas; podem ser desligadas na janela.

Durante o desenvolvimento, os testes encontraram uma corrida ao fechar a porta serial no Windows e uma possibilidade de duas instâncias reutilizarem a porta local do aplicativo. A finalização da leitura foi ajustada e a porta local passou a ser exclusiva; a segunda abertura foi verificada após a correção. O build separado `mic_test` também passou, sem substituir o firmware principal instalado.

## Limites do que foi comprovado

- **Não houve captura nova pelo botão nesta etapa.** O teste integrado usou uma gravação física anterior e um comando sintetizado. A montagem de microfone já havia sido testada nas etapas anteriores.
- **Não foi conectado um celular real.** O HTTP/DNS foi exercitado pela própria placa através da interface AP, e o formulário real foi testado em navegador com servidor simulado. Isso não valida o comportamento específico de Android/iOS ao entrar numa rede sem internet.
- **A reprodução foi concluída pelo dispositivo de áudio padrão do Windows.** Foi gerado um WAV com sinal e o motor confirmou a reprodução; não houve medição acústica externa do alto-falante.
- A calculadora foi aberta de verdade. Os outros destinos têm implementação com caminhos/URLs fixos e testes da lógica; não foram todos abertos fisicamente nesta rodada.
- Voz e ações no PC exigem aplicativo ativo. Sem PC, a resposta permanece na tela; não há alto-falante físico nesta montagem. API e transcrição ainda exigem internet e cotas disponíveis.
- Consultas HTTPS continuam bloqueantes, com timeouts. Não foi implementado streaming, teclado físico, comandos gerais do sistema ou saída de áudio diretamente na placa.

## Referências técnicas

- [Espressif: Wi-Fi AP/STA](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html)
- [Espressif: exemplo de DNS para portal](https://github.com/espressif/arduino-esp32/blob/master/libraries/DNSServer/examples/CaptivePortal/CaptivePortal.ino)
- [Microsoft: SpeechSynthesizer](https://learn.microsoft.com/en-us/dotnet/api/system.speech.synthesis.speechsynthesizer?view=netframework-4.8.1)

## English

Version 0.5.0 was built, flashed with hash verification and tested autonomously. The physical ESP32 processed prerecorded microphone audio through Groq and the real response was played by Windows. A synthesized Portuguese command opened Calculator; a two-turn chat retained the provided name. USB validation, portal HTTP/DNS on the board, the mobile form, host regression tests and the running companion's single-instance behavior passed.

No fresh live microphone/button recording or real phone connection was tested in this stage. Playback completion was reported by Windows, without external acoustic measurement. Other allowed apps/sites were not all physically launched. PC features require the active Windows companion; standalone audio output and a physical keyboard remain absent. Keys remain in unencrypted NVS and cloud quotas still apply.
