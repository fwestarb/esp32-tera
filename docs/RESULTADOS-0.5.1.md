# Resultados — correção 0.5.1

## Problema observado

Em 10/10/2026, o aplicativo antigo indicava USB conectado, mas a ESP32 não devolvia nenhum dado: nem status, nem log de reset, nem sincronização com o bootloader pelo esptool. A COM3 continuava presente. O Windows negou a tentativa de reiniciar o dispositivo USB. **Depois que o usuário reconectou o cabo, a placa voltou a responder.** Isso comprova a recuperação da comunicação; não identifica por si só a causa elétrica/USB exata do travamento.

Também foram encontrados problemas no código: a conexão confiava numa sessão antiga sem conferir respostas recentes; uma confirmação atrasada podia provocar novo pareamento; identificadores de ações reiniciados junto com a placa podiam ser ignorados como duplicados; mensagens de manutenção eram enviadas durante consultas bloqueantes.

## Correções instaladas

- Firmware **0.5.1**, gravado na TTGO com hash conferido e configuração preservada.
- Heartbeats confirmados, identificação do boot e renovação da sessão, sem conservar indevidamente ações do boot anterior.
- Aplicativo detecta falta de resposta, fecha a porta e tenta reconectar. Não acumula heartbeats durante gravação/consulta.
- Watchdog da tarefa principal de 120 segundos, com alimentação por etapa. A inscrição foi confirmada no status; não foi provocado um travamento artificial para demonstrar o disparo.
- X da janela minimiza. **Encerrar assistente** desliga a integração.
- Voz padrão **Faber, neural local em português**, pelo Piper no PC. Seletor com Faber, Daniel e Maria; exemplo e preferência persistente. Sem outra chave/API de voz.

## Verificado após a correção

| Teste | Resultado |
|---|---|
| Build/upload | Passou; hash da flash conferido — [build](BUILD-0.5.1.json) |
| USB físico | 10 verificações passaram — [USB](TESTES-USB-0.5.1.json) |
| Regressões PC | 21 testes passaram — [PC](TESTES-PC-0.5.1.json) |
| Calculadora + Faber | Abertura real e reprodução concluída antes e depois de reset da ESP32 |
| Sessão após reinício | Novo boot detectado; ação de ID 1 voltou a funcionar; heartbeats por pelo menos 16 segundos |
| Áudio de comando | “Abra a calculadora.” sintetizado: Groq HTTP 200, abertura e confirmação Faber |
| Chat + nova voz | Resposta real “Tera está funcionando corretamente.” reproduzida pelo Piper — [chat](TESTE-CHAT-FALA-0.5.1.json) |

[Relatório de reinício, ações e voz](TESTE-RECUPERACAO-0.5.1.json). [Síntese WAV e hash do modelo](TESTE-PIPER-0.5.1.json). O aplicativo permanece conectado com Faber selecionada.

Não houve nova gravação do usuário; o reconhecimento usou áudio sintetizado. A reprodução foi confirmada pelo motor no dispositivo padrão do Windows, sem medição acústica externa. Os testes não garantem ausência de toda falha futura de cabo, alimentação ou driver.

## Voz e distribuição

Piper 1.4.2 e o modelo pt_BR-faber-medium são instalados em ambiente isolado no computador e ficam fora do Git. O modelo tem aproximadamente 63 MB e não ocupa a ESP32. O instalador confere SHA-256. Piper usa GPL-3.0; o cartão Faber informa dataset CC0. Não foram incorporados binários desses componentes no repositório.

Fontes: [Piper oficial](https://github.com/OHF-Voice/piper1-gpl), [modelo Faber](https://huggingface.co/rhasspy/piper-voices/tree/main/pt/pt_BR/faber/medium), [cartão do modelo](https://huggingface.co/rhasspy/piper-voices/blob/main/pt/pt_BR/faber/medium/MODEL_CARD).

## English

The old app showed a connection even though the ESP32 returned no USB data. Communication returned after reconnecting the cable. Version 0.5.1 checks recent responses, renews sessions after reboot, avoids stale action IDs and stops maintenance traffic during blocking requests. A 120-second main-task watchdog is registered; its timeout was not deliberately triggered.

Offline Portuguese Piper Faber speech is available with selectable Windows alternatives. Physical tests opened Calculator and played Faber before/after reset, transcribed a synthesized command through Groq, and spoke a real chat reply. 21 host regressions and 10 physical USB checks passed. No new live user recording or external acoustic measurement was required.
