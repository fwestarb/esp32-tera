# Pesquisa inicial — 03/10/2026

**Escolha inicial: OpenRouter para texto com chave de cada usuário.** Motivos: configuração já conhecida no GhostReply, API HTTPS e seleção de modelos gratuitos. O roteador `openrouter/free` pode variar modelo e disponibilidade. Ele não garante voz de entrada/saída gratuita.

- [OpenRouter limites](https://openrouter.ai/docs/api_reference/limits): contas gratuitas têm limites próprios; a página de preços indica 50 pedidos/dia no plano gratuito. Verificar cotas novamente durante implementação.
- [Free router](https://openrouter.ai/openrouter/free): seleção automática entre modelos gratuitos compatíveis.
- [Hugging Face preços](https://huggingface.co/docs/inference-providers/pricing): usuários gratuitos recebem US$ 0,10/mês em créditos, sujeito a alteração. Inferência hospedada de modelos abertos não equivale a uso ilimitado gratuito. Por isso não é a primeira opção de texto deste protótipo.
- [OpenRouter entrada de áudio](https://openrouter.ai/docs/guides/overview/multimodal/audio): pode encaminhar áudio a modelos compatíveis. O catálogo público encontrou modelos `:free` com áudio. Porém o endpoint gratuito [NVIDIA Nemotron Omni](https://openrouter.ai/nvidia/nemotron-3-nano-omni-30b-a3b-reasoning:free) pede explicitamente que não se enviem vozes/dados pessoais; ele não foi escolhido para a gravação do usuário. Outros candidatos, como `thinkingmachines/inkling-small:free`, ainda exigem avaliação das condições, qualidade em português e memória da placa.
- [OpenRouter saída de voz](https://openrouter.ai/docs/guides/overview/multimodal/tts): endpoint próprio `/audio/speech`. O catálogo específico de fala retornou [Fish Audio S2.1 Pro Free](https://openrouter.ai/fish-audio/s2.1-pro-free:free), candidato gratuito para protótipos. Sem garantias de disponibilidade/latência e ainda sem teste em português, chamada autenticada ou reprodução na placa. A escolha final dependerá desses testes e do formato suportado pelo hardware.

Implementação pretendida: texto primeiro; áudio de entrada com modelo multimodal ou transcrição conforme testes; áudio de saída com provedor/modelo compatível, opcional. Caso não exista opção gratuita viável, apresentar essa limitação e escolher o caminho com o usuário. Nunca acionar modelo pago automaticamente.

As informações vêm de documentação oficial e do catálogo público; não foi feita chamada autenticada ou consumida nenhuma chave/cota durante esta pesquisa.
