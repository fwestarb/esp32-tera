# Groq gratuito

1. Entre em https://console.groq.com/keys e crie uma chave pessoal (gsk_...).
2. Mantenha a conta no plano Free. O firmware nao verifica seu plano de faturamento.
3. Na interface Tera, selecione groq, preencha Wi-Fi 2,4 GHz, senha e a chave Groq; salve na placa. A chave OpenRouter nao funciona na Groq.
4. Aguarde Wi-Fi/relogio e envie uma pergunta curta. Nao envie a chave pelo chat nem publique no Git.

Modelo: openai/gpt-oss-20b. Documentacao de 06/10/2026: Free com 30 pedidos/minuto, 1.000/dia, 8.000 tokens/minuto e 200.000 tokens/dia. Verifique os limites exatos da sua conta; qualquer um desses limites pode interromper as consultas. Nao e ilimitado.

Firmware 0.3.0 compilado e instalado com hash verificado; dez testes USB passaram. HTTPS com Groq passou na TTGO: HTTP 401 esperado ao consultar /openai/v1/models sem chave. Isso valida TLS, nao geracao de texto. A resposta autenticada aguarda sua chave Groq.

Credenciais OpenRouter atuais preservadas ate salvar a nova configuracao. Evidencias: TESTE-GROQ-TLS.json e TESTES-USB-0.3.0.json.

Fontes oficiais:
- https://console.groq.com/docs/rate-limits
- https://console.groq.com/docs/quickstart
- https://console.groq.com/docs/billing-faqs
- https://console.groq.com/docs/api-reference

## English

Select Groq in the USB tool and save your Wi-Fi credentials and personal Groq key. Keep the account on Free; the firmware does not verify billing status. Model: openai/gpt-oss-20b. Physical-board TLS and protocol checks passed; authenticated chat awaits a user-provided key.

## Atualizacao de voz 09/10/2026

Firmware 0.4.0 acrescenta Whisper large v3 turbo para transcrever portugues. Autenticacao, transcricao e resposta GPT OSS passaram na TTGO usando uma gravacao anterior conhecida; veja RESULTADOS-VOZ-0.4.0.md. O plano Free tem cotas separadas de texto e audio. Cada pergunta por voz usa dois pedidos.
