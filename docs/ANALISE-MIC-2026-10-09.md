# Analise do teste da GPIO32 — 09/10/2026

## Resultado

Ha evidencia de captacao pelo circuito do microfone. O usuario confirmou que falou perto dele sem tocar nos fios/circuito durante a captura. Houve picos de variacao claramente acima do nivel de fundo, com retorno a valores baixos e sem saturacao nas amostras observadas. Isso e um resultado positivo no teste de nivel, mas nao valida ainda inteligibilidade de uma gravacao nem reconhecimento de fala.

| Medida | Resultado (unidades ADC) |
| --- | --- |
| Janelas recebidas | 130, com 800 amostras por janela |
| RMS mediano | 3,805 |
| Maior RMS | 32,88 (aproximadamente 8,6 vezes a mediana) |
| Maior pico a pico | 192 em uma escala de 0 a 4095 |
| Media DC dos trechos | Aproximadamente 1934 |
| Janelas acima do limiar de atividade | 8 |
| Amostras nos extremos do ADC | 0 |

A polarizacao ficou longe dos extremos. O maior pico ocorreu em 20,641 segundos relativos ao inicio da captura; outros picos ocorreram perto de 19 a 22 segundos. As leituras predominantes de RMS permaneceram perto de 3,8. A relacao com a fala depende da confirmacao do usuario; nao houve um marcador sincronizado de inicio/fim de fala.

## Limites da captura

Foram solicitados 30 segundos de captura, mas a primeira linha valida chegou somente aos 14,688 segundos. O log cobre cerca de 15,3 segundos de leituras validas, ate 30,016 segundos. Nao e correto apresentar esse arquivo como 30 segundos completos de sinal. Depois da primeira linha, a maior distancia entre linhas foi 0,125 segundo. A causa da demora inicial nao foi determinada.

Este firmware transmite estatisticas por janela, nao a forma de onda original. Nao permite avaliar espectro, zumbido, inteligibilidade ou qualidade de voz. Os valores nao sao decibeis nem uma medida calibrada de volume. A ausencia de saturacao se aplica somente a esta captura.

## Proximo passo

Gravar um trecho curto de PCM com este circuito e ouvir/verificar o audio antes de integrar transcricao e IA. Nao aumentar o ganho somente com base nestes numeros: primeiro avaliar a gravacao.

Arquivos: MIC-LOG-2026-10-09.json (dados e resumos) e MIC-LOG-2026-10-09.csv (leituras para planilha).
