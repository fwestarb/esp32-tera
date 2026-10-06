# Resultado da etapa — 06/10/2026

## Atualizacao Groq — 0.3.0

Adicionados selecao de provedor e configuracao Groq. Modelo GPT-OSS 20B, a ser usado com conta Free. Configuracao OpenRouter anterior preservada. A mensagem 429 agora distingue limite/capacidade de consumo pessoal presumido.

Firmware compilado/instalado: RAM estatica 46.500 bytes; flash 965.437 bytes. Dez testes USB passaram. HTTPS Groq passou na placa (HTTP 401 esperado sem chave). Geracao autenticada ainda aguarda chave Groq pessoal. Veja GROQ.md, TESTE-GROQ-TLS.json e TESTES-USB-0.3.0.json.

## Evidência obtida

- Placa conectada em COM3: ESP32-D0WDQ6, revisão 1.0, compatível com TTGO T-Display original.
- Backup completo anterior: 4.194.304 bytes, leitura concluída pelo esptool antes da gravação.
- SHA-256 do backup: `2BD8886092805403B078D73F2F376FABF0E33B596893184049901C5E2179D6DC`.
- Backup local: `private-backups/before-text-milestone.bin`, ignorado pelo Git; contém memória potencialmente sensível.
- Build/upload concluídos. esptool confirmou o hash das regiões escritas. A NVS não foi apagada.
- Firmware retornou versão 0.2.0 e 245.992 bytes livres de heap no teste inicial, sem Wi-Fi conectado.
- 11 verificações passaram: inicialização/estado, JSON inválido, comando desconhecido, configuração incompleta, mensagem vazia, mensagem longa, linha serial excessiva, rejeição de modelo pago, rejeição de injeção na chave, manutenção do estado após comandos inválidos e erro honesto sem chave.
- Evidência reproduzível em `TESTES-USB.json` e `tools/test_device.py`.
- Ferramenta Python passou pela compilação de sintaxe e pela criação da janela/inicialização do loop de eventos em teste local. O fluxo completo da interface com credenciais reais ainda precisa ser utilizado.

## Limites reais

A implementação de HTTPS/API existe, mas não foi demonstrada uma resposta real: falta configurar Wi-Fi e chave pessoal na ferramenta. Não foram verificadas captura de microfone, reprodução de áudio, teclado ou autonomia de entrada sem PC. Sem essas peças, o texto entra por USB. Não há histórico entre perguntas.

As respostas são limitadas a 320 tokens e filtradas em um documento JSON com capacidade de 8 KiB. Mensagens maiores que 1024 bytes e comandos maiores que 3072 bytes são recusados. Chamadas de rede bloqueiam o loop enquanto aguardam; o limite de leitura não garante duração total exata da consulta. Créditos/limites do OpenRouter continuam sujeitos ao provedor.

## Próximo teste concreto

Abra `configurar.cmd`, configure Wi-Fi/chave e envie: “Explique em uma frase o que é uma ESP32”. Confirme a resposta na TTGO e na interface. Depois reinicie a placa para verificar a persistência e repita. Somente após isso podemos marcar Wi-Fi/API como validados e avançar para entrada de teclado ou microfone.

## Certificados

Raízes obtidas de https://curl.se/ca/cacert.pem em 06/10/2026 (121 certificados). Bundle gerado com o utilitário oficial Espressif IDF v4.4.6: https://github.com/espressif/esp-idf/blob/v4.4.6/components/mbedtls/esp_crt_bundle/gen_crt_bundle.py (Apache 2.0, aviso preservado no arquivo). O firmware verifica os certificados e sincroniza o relógio antes de usar HTTPS. Atualizações futuras devem atualizar o bundle quando necessário.


## Correção de Wi-Fi — firmware 0.2.1

A placa confirmou configuração persistida, mas retornou motivo ESP32 201 (NO_AP_FOUND: rede não encontrada). Isso não identifica senha incorreta; verificar nome exato, sinal e disponibilidade em 2,4 GHz. Não depende de microfone, teclado ou alto-falante.

Correções: estado detalhado após 20 segundos, mensagem de espera durante conexão, motivo de desconexão, botão de diagnóstico de rede e envio à IA bloqueado na interface enquanto o Wi-Fi não conecta. A busca desconecta temporariamente a estação e retoma a conexão sem apagar a configuração. Interface iniciou sem erro no teste local. Evidência em DIAGNOSTICO-WIFI.json.

Teste adicional na versão final: busca 2,4 GHz executada com sucesso; a rede configurada não apareceu. Dez testes de protocolo passaram novamente com a configuração do usuário preservada (TESTES-USB-0.2.1.json). Falha de Wi-Fi confirmada; API ainda não foi consultada.

## Correção HTTPS — firmware 0.2.2

Falha reproduzida na TTGO: HTTPClient -1, TLS -12288 (`X509 ... vrfy callback failed`). A verificação da cadeia pelo bundle anterior falhou para a cadeia atual do OpenRouter. Foram usadas as raízes GTS R1/R3/R4 presentes no conjunto Mozilla já obtido, em PEM através de setCACert. A validação de certificado/hostname continua ativa. Verificação independente no computador com somente essas raízes também passou.

Resultado físico após correção: conexão HTTPS concluída e resposta real da API recebida. O roteador `openrouter/free` devolveu `User Safety: safe` para a pergunta de teste; esse resultado não valida um chat útil. Para evitar essa escolha automática, o firmware foi fixado em `google/gemma-4-26b-a4b-it:free`, que a API pública de modelos listou com preço de entrada/saída zero. A consulta a esse modelo retornou HTTP 429. Não há evidência suficiente para distinguir limite da conta de saturação do provedor. Não foram comprados créditos e não há fallback para modelos pagos.

O problema TLS relatado foi corrigido. Conversa com o modelo atual permanece bloqueada por limite/capacidade do serviço gratuito; não foi validada uma resposta útil dele. Evidência da última chamada em TESTE-IA-REAL.json. Firmware instalado com hash verificado; RAM estática 46.484 bytes, flash 963.281 bytes (73,5%). As credenciais existentes foram preservadas.

Quando o serviço liberar novas consultas, conectar a interface, aguardar Wi-Fi/horário e repetir a pergunta. Uma chave de outra conta com disponibilidade também pode ser configurada localmente, se o usuário já a tiver; nunca colar chaves em chat ou em Git.
