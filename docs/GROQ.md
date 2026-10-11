# Groq — configuração Alpha 1.0

Crie sua chave pessoal em https://console.groq.com/keys e mantenha o plano gratuito se não quiser pagar. O sistema não consulta faturamento nem altera o plano.

Selecione Groq na Tera, configure Wi-Fi 2,4 GHz e chave, e escolha **openai/gpt-oss-120b** (padrão) ou **openai/gpt-oss-20b**. A escolha fica na placa e pode ser alterada sem reinstalar firmware. Aguarde Wi-Fi e relógio para HTTPS. Não envie sua chave pelo chat ou publique no Git.

A entrada de voz usa **whisper-large-v3-turbo**, separadamente do modelo de conversa. Cada pergunta falada normalmente usa uma transcrição e uma consulta de chat. Aberturas reconhecidas localmente usam só a transcrição. A voz feminina Dora é gerada no PC, sem API de fala.

Ambos os modelos de chat responderam em consultas autenticadas pela TTGO nos testes de 11/10/2026. As cotas dependem do modelo e da conta; confira https://console.groq.com/docs/rate-limits. HTTP 429 pode indicar cota ou capacidade. A aprovação dos testes não comprova uso ilimitado.

OpenRouter permanece uma alternativa de texto com chave própria e modelos gratuitos. Não fornece transcrição do microfone nesta alpha.

## English

Create your personal Groq key and use the Free plan if you do not want billing. Tera does not change or inspect billing. Choose GPT-OSS 120B or 20B; both were tested with actual authenticated requests on the ESP32. The selected model persists without reflashing. Whisper large-v3-turbo handles speech input independently. Offline Dora speech runs on the PC. Cloud quotas remain account/model dependent; 429 can indicate rate limits or capacity.
