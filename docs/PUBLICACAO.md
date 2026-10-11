# Publicação / Publication

## Português

- [Site com instalação e documentação](https://fwestarb.github.io/esp32-tera/)
- [Código do esp32-tera](https://github.com/fwestarb/esp32-tera)
- [Alpha 1.0, ZIP e SHA-256](https://github.com/fwestarb/esp32-tera/releases/tag/v1.0.0-alpha.1)

O site atual usa React + Vite com seis componentes React Bits. Execute `npm ci` e `npm run build` em `website`; o build atualiza `docs/index.html` e `docs/ui/assets`. Consulte [desenvolvimento do site](../website/README.md). GitHub Pages publica a pasta `docs` da branch `main`. O arquivo `.nojekyll` mantém os arquivos estáticos. O fundo tem base sólida e efeitos discretos opcionais; as animações são visuais, não medições da IA. A foto mostra o protótipo real em um teste anterior.

A página tem português/inglês, botões com resposta visual, cópia de comandos, perguntas frequentes, instalação e configuração. O idioma escolhido é salvo apenas no navegador. Não coleta chaves ou configura a placa: essa tarefa pertence ao aplicativo USB e ao portal da própria ESP32.

### Verificações do site

Edge headless com Playwright: português e inglês nas larguras 320, 390, 768 e 1440 px, ausência de rolagem horizontal, tradução completa, persistência do idioma, cópia para a área de transferência, expansão das dúvidas, imagens carregadas e preferência de movimento reduzido. Sem erros de JavaScript. Capturas revisadas visualmente. [Relatório da página inicial](TESTE-SITE-ALPHA.json). A revisão React está em [TESTE-SITE-REACT.json](TESTE-SITE-REACT.json), incluindo galeria, WebGL, download real, cancelamento e erros de integridade.

Para repetir: sirva `docs` em `http://127.0.0.1:8808` e execute `node tools/test_site.cjs` com Playwright e Microsoft Edge instalados. `TERA_SITE_URL` permite testar outro endereço. As capturas ficam em `private-backups/site-check-v2`, fora do Git.

O ZIP contém firmware, fontes e scripts explicitamente selecionados. Modelos de voz, ambientes Python, gravações, credenciais e dumps de NVS não são distribuídos. SHA-256 acompanha o pacote e o manifesto dos binários. Leia [instalação](INSTALACAO-ALPHA.md) e [resultados do sistema](RESULTADOS-ALPHA-1.0.md) para requisitos e limites.

### Verificação após publicar

O endereço público respondeu HTTP 200 e passou pelos mesmos testes do site local. O ZIP e o arquivo SHA-256 foram baixados do GitHub e comparados integralmente com os arquivos locais. GitHub Pages informou build concluído. [Evidência da publicação](VERIFICACAO-PUBLICACAO.json). Não foram repetidos testes do microfone ou alterado o firmware nesta publicação.

### Atualizar depois

Mudanças em `docs` publicadas na branch `main` atualizam o site. Para uma nova versão, atualize a versão do firmware, valide o sistema, gere um novo pacote com `tools/package_alpha.py` e publique outra tag/release; atualize o nome/hash em `website/src/content.js`, o ZIP em `docs/downloads` e gere o build. Mantenha as versões anteriores disponíveis.

## English

The static site is published from `main` → `/docs` on GitHub Pages. It now uses React + Vite and six supplied React Bits components. Run `npm ci` and `npm run build` inside `website` to update the static Pages output. See the website README for build instructions, licensing and the pending Pro component. The tree animation is a brand illustration, and the photograph shows a real earlier prototype test. Portuguese/English selection, responsive layout, command copying and reduced-motion support are included. No API keys are collected by the public website.

Playwright checks in headless Edge cover both languages at 320, 390, 768 and 1440 px, overflow, translations, language persistence, clipboard, FAQ expansion, image loading and reduced motion, with no JavaScript errors. Screenshots were visually reviewed. See the linked JSON report and `tools/test_site.cjs` to repeat the checks.

The release includes an explicit selection of source files and firmware, with SHA-256 integrity files. Voice models, environments, recordings, secrets and NVS dumps are excluded. Installation requires Python 3.12 on Windows and a personal provider key. Free APIs have quotas. Firmware/app validation and untested hardware cases are documented separately in the Alpha results.

The deployed website also passed the browser checks with HTTP 200. Release assets were downloaded from GitHub and matched the local files byte for byte. See [public deployment verification](VERIFICACAO-PUBLICACAO.json). No firmware changes or new microphone tests were performed during website publication.

## Revisão React Bits

A revisão com seis componentes, base sólida e textos objetivos também passou nos testes após publicação: [relatório público React](TESTE-SITE-REACT-PUBLICO.json). Inclui download real da cópia da Alpha, SHA-256, cancelamento, rejeição de arquivo corrompido, tratamento de falha de rede, galeria por teclado e fallback sem WebGL. Dappled Light permanece pendente por ausência de licença. O firmware e a release Alpha permanecem os mesmos.

The React Bits revision passed the same checks on the public URL, including a real download and integrity/error handling. See the linked public report. Dappled Light is pending a Pro license. Firmware and the Alpha release are unchanged.


## Revisão visual de 11/10/2026

Árvore fornecida pelo autor com HalftoneReveal ao mover o cursor ou tocar; botão de alternância acessível por teclado. Três fotos novas, FlexCarousel com curvatura e dispersão, SideRays mais visível, FoldText, GlideSelect e LatticeLoader durante download real. Fontes Oi e STIX Two Text, navegação em pílulas e fundo sólido com pontos SVG. Dappled Light Pro permanece pendente de licença.

Verificação automática: PT/EN em 320, 390, 768 e 1440 px, sem rolagem horizontal; carregamento das fontes, mudança real dos pixels da árvore ao mover o cursor, galeria por teclado, download com SHA-256, cancelamento e falhas, redução de movimento e alternativa sem WebGL. Relatórios: TESTE-SITE-REACT.json e TESTE-EFEITOS-VISUAIS.json.
