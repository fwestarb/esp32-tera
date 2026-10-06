# Resultado da etapa — 06/10/2026

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

