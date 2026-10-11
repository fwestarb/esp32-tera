# Estado atual — Alpha 1.0

A implementação atual está documentada em README.md, INSTALACAO-ALPHA.md e RESULTADOS-ALPHA-1.0.md. Já existem configuração USB/celular, chat real com modelos selecionáveis, entrada de voz GPIO32/35, voz feminina no PC e abertura de aplicativos permitidos. Não há streaming; a conversa mantém apenas a última troca. O modelo de chat não é executado na ESP32.

Repositório, site e pacote Alpha publicados com autorização do autor: consulte [PUBLICACAO.md](PUBLICACAO.md).

Próximas etapas, fora desta alpha:

1. Testes em um celular físico e em outro Windows.
2. Teclado opcional quando a peça estiver disponível.
3. Saída de áudio na placa quando os componentes forem definidos.
4. Melhorar responsividade durante HTTPS e avaliar streaming conforme os limites de memória.

Nenhuma compra é necessária para a preparação local atual.

## Histórico do planejamento (não é a lista de recursos prontos)

# Sequência de execução

1. **Fundação:** registrar requisitos, pesquisar provedores e hardware, criar estrutura PlatformIO, tela base e contrato de configuração. Compilar sem gravar a placa.
2. **Interface Tera:** telas de configuração, edição, conversa e erros; rolagem para respostas longas; árvore com folhas associadas ao progresso real da resposta. Paleta preta/cinza com verde claro, escuro e musgo.
3. **Configuração por USB:** ferramenta local no computador para gravar firmware e, em seguida, configurar Wi-Fi, chave pessoal, modelos e periféricos via serial. Salvar configurações em NVS, mascarar segredos, permitir alterar/apagar credenciais e validar campos. Não modificar nem recompilar fonte para inserir chave. Instalação inicial via PlatformIO; instalador amigável entra nesta etapa.
4. **Chat por Wi-Fi:** cliente HTTPS com validação de certificado, relógio sincronizado, requisições limitadas, histórico curto e respostas em fluxo. Chave de cada usuário enviada diretamente ao provedor escolhido, sem servidor pessoal obrigatório. OpenRouter gratuito como opção inicial de texto; bloquear seleção paga no modo gratuito. Distinguir chave inválida, cota, falta de rede e timeout.
5. **Entrada sem teclado físico:** durante desenvolvimento, mensagens por serial para testar o chat. Depois, entrada pelo microfone ou pelo teclado opcional. O computador não será necessário no uso final.
6. **Teclado CardKB:** driver I²C, detecção opcional, editor com Enter/Backspace e navegação. Validação de lógica sem peça; teste elétrico e físico somente com teclado disponível.
7. **Voz de entrada:** microfone analogico com amplificador discreto em GPIO32. Segurar GPIO35 para captar, soltar para enviar; limite de 10 segundos. Groq Whisper para transcricao e GPT OSS para resposta. Validar compreensao em portugues e HTTPS com memoria de audio ocupada antes de declarar funcional.
8. **Voz de saída opcional:** identificar amplificador/saída; selecionar provedor/formato, reproduzir por buffers pequenos, controlar volume e permitir desligar. Falha de voz não deve impedir resposta na tela. Gratuidade e português precisam ser comprovados para o modelo escolhido.
9. **Validação integrada:** inicialização/configuração persistente, Wi-Fi, conversa real, recuperação de falhas, memória com TLS/áudio, botões, teclado e áudio instalado. Registrar compilação separadamente de testes físicos.
10. **Entrega GitHub:** tutorial PT/EN, diagrama específico das peças reais, configuração de chave por usuário, demonstração, limitações e publicação após a primeira versão funcional. Não publicar segredos ou apresentar recursos pendentes como prontos.

## Decisões confirmadas

- Placa original, sem PSRAM; sem dependência de PC após configurar.
- Pressionar/segurar para falar, soltar para enviar.
- Resposta sempre na tela; saída de voz configurável.
- Microfone analogico ja montado/testado; saida de audio e teclado ainda sem pecas identificadas para integracao.
- Um usuário fornece sua própria API key. Cotas continuam pertencendo à conta do provedor; várias chaves da mesma conta não significam cotas independentes.

## Arquitetura pretendida

```text
Instalador local --USB/serial--> ESP32: NVS (Wi-Fi, chave, opções)
                                   |
Botões / CardKB / microfone --> aplicação --> HTTPS --> provedor de IA
                                   |
                            tela / saída de áudio opcional
```

NVS padrão não deve ser descrito como armazenamento criptografado. Proteção de flash/chaves requer uma decisão separada; não ativar eFuses irreversíveis automaticamente. A chave não será exibida em logs ou exportada por padrão.

## Pendências externas

Saida de audio, confirmacao eletrica do teclado e demonstracao independente do computador com o botao real. A placa esta conectada e a captacao analogica foi validada. Groq Free foi escolhido para a etapa de voz, sujeito a limites.

## Atualizacao 09/10/2026 — voz 0.4.0

Captacao de fala em GPIO32 e transcricao local dos testes anteriores funcionaram. O firmware integrado agora inclui botao GPIO35, envio WAV para Groq, filtro de sinal baixo/saturacao, transcricao com verificacao de probabilidade de silencio e resposta na tela. O envio independe do PC.

A validacao integrada esta registrada separadamente em `TESTE-AUDIO-MEMORIA-0.4.0.json` e `TESTE-VOZ-REAL-0.4.0.json`. Erros reais de fragmentacao de RAM na alocacao de audio/TLS foram encontrados durante os testes; veja VOZ.md. Transcricao e resposta reais foram obtidas pela TTGO com replay de uma gravacao anterior. Falta confirmar nova fala pelo botao e uso com PC desligado; ver RESULTADOS-VOZ-0.4.0.md.

## Atualização 06/10/2026

A etapa de USB/configuração e o código de Wi-Fi/chat de texto estão implementados. Build, upload e validações USB foram testados na placa. Conexão Wi-Fi e resposta real da IA ainda aguardam credenciais e teste do usuário; veja RESULTADOS.md. A entrada atual é pelo computador via USB. Teclado e áudio permanecem pendentes; ainda não há chat autônomo completo.

## Atualização 10/10/2026 — versão 0.5.0

Implementados: portal Wi-Fi de configuração com senha temporária, aplicativo Windows com fala local, abertura de programas/sites por comandos definidos e contexto da última troca em RAM. A entrada física continua em GPIO32/35. O portal e a integração PC têm testes automatizados; veja RESULTADOS-0.5.0.md. O PC só é necessário para a voz de saída e as ações de abertura, além da instalação.

Próximas etapas: teclado opcional quando disponível; saída de áudio física quando as peças forem definidas; maior responsividade durante HTTPS. Captura nova pelo botão e conexão de um celular real não foram acrescentadas aos testes desta etapa, que não exigiu participação do usuário.
