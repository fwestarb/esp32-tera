# ESP32 Tera 1.0.0-alpha.3

Atualização do companion Windows: botão **Salvar diagnóstico** exporta um JSON local para suporte, com estados de conexão, ações, voz, Wi-Fi e versão conhecida do firmware. Chaves, mensagens, SSID, identificadores de sessão e erros livres não são exportados. O arquivo não é enviado automaticamente.

O pacote inclui o módulo novo e testes. Firmware incluído permanece **1.0.0-alpha.2**, byte a byte igual ao da release anterior; esta mudança não exige regravar a placa. Quem usa o firmware anterior à alpha.2 deve atualizar firmware e companion conforme as notas da alpha.2.

Use a pasta nova extraída e execute configurar.cmd. Modelos de voz e dependências continuam externos. Não copie arquivos novos por cima de um processo antigo em execução; encerre o assistente pelo botão antes de atualizar.

Validação: 24 testes de bridge, 6 testes de suporte e empacotamento com fixtures. Janela Tk e callback do novo botão verificados de forma isolada com USB, fala e diálogos simulados. Não houve nova validação física de placa, microfone ou IA.
