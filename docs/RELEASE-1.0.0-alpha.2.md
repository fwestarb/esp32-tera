# ESP32 Tera 1.0.0-alpha.2

Publicação de 01/11/2026 com as correções da Vibe Check. Atualize firmware e companion juntos. A versão anterior permanece disponível.

- Comandos após voz demorada exigem confirmação fresca da mesma sessão em até dois segundos; timeout e sessão incorreta não executam.
- Rede aberta limpa a senha no parser e no portal.
- Validação local: 24 testes da integração PC, build PlatformIO e portal no Chrome com API simulada.
- Não houve teste físico novo de placa, microfone, voz ou Groq. O build não equivale a validar esses fluxos.
- Instalação: extraia o pacote e siga INSTALACAO-ALPHA.md; gravação preserva NVS. Modelos de voz continuam sendo baixados separadamente.
