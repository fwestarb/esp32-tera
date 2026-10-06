# Sequência de execução

1. **Fundação:** registrar requisitos, pesquisar provedores e hardware, criar estrutura PlatformIO, tela base e contrato de configuração. Compilar sem gravar a placa.
2. **Interface Tera:** telas de configuração, edição, conversa e erros; rolagem para respostas longas; árvore com folhas associadas ao progresso real da resposta. Paleta preta/cinza com verde claro, escuro e musgo.
3. **Configuração por USB:** ferramenta local no computador para gravar firmware e, em seguida, configurar Wi-Fi, chave pessoal, modelos e periféricos via serial. Salvar configurações em NVS, mascarar segredos, permitir alterar/apagar credenciais e validar campos. Não modificar nem recompilar fonte para inserir chave. Instalação inicial via PlatformIO; instalador amigável entra nesta etapa.
4. **Chat por Wi-Fi:** cliente HTTPS com validação de certificado, relógio sincronizado, requisições limitadas, histórico curto e respostas em fluxo. Chave de cada usuário enviada diretamente ao provedor escolhido, sem servidor pessoal obrigatório. OpenRouter gratuito como opção inicial de texto; bloquear seleção paga no modo gratuito. Distinguir chave inválida, cota, falta de rede e timeout.
5. **Entrada sem teclado físico:** durante desenvolvimento, mensagens por serial para testar o chat. Depois, entrada pelo microfone ou pelo teclado opcional. O computador não será necessário no uso final.
6. **Teclado CardKB:** driver I²C, detecção opcional, editor com Enter/Backspace e navegação. Validação de lógica sem peça; teste elétrico e físico somente com teclado disponível.
7. **Voz de entrada:** identificar o microfone antes de escolher driver/pinos. Segurar GPIO35 para captar, soltar para enviar; limite inicial de 5 segundos, ajustado após medir memória. Investigar envio direto a modelo gratuito multimodal ou serviço de transcrição. Validar compreensão em português antes de declarar funcional.
8. **Voz de saída opcional:** identificar amplificador/saída; selecionar provedor/formato, reproduzir por buffers pequenos, controlar volume e permitir desligar. Falha de voz não deve impedir resposta na tela. Gratuidade e português precisam ser comprovados para o modelo escolhido.
9. **Validação integrada:** inicialização/configuração persistente, Wi-Fi, conversa real, recuperação de falhas, memória com TLS/áudio, botões, teclado e áudio instalado. Registrar compilação separadamente de testes físicos.
10. **Entrega GitHub:** tutorial PT/EN, diagrama específico das peças reais, configuração de chave por usuário, demonstração, limitações e publicação após a primeira versão funcional. Não publicar segredos ou apresentar recursos pendentes como prontos.

## Decisões confirmadas

- Placa original, sem PSRAM; sem dependência de PC após configurar.
- Pressionar/segurar para falar, soltar para enviar.
- Resposta sempre na tela; saída de voz configurável.
- Microfone, saída e teclado com opções independentes; peças de áudio ainda desconhecidas.
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

Modelos dos componentes de áudio, confirmação elétrica do teclado, placa conectada para validação física e provedor/modelo de voz gratuito viável. O desenvolvimento da base e do chat textual pode avançar antes dessas respostas.

## Atualização 06/10/2026

A etapa de USB/configuração e o código de Wi-Fi/chat de texto estão implementados. Build, upload e validações USB foram testados na placa. Conexão Wi-Fi e resposta real da IA ainda aguardam credenciais e teste do usuário; veja RESULTADOS.md. A entrada atual é pelo computador via USB. Teclado e áudio permanecem pendentes; ainda não há chat autônomo completo.
