# Publicação / Publication

## Português

- [Site com instalação e documentação](https://fwestarb.github.io/esp32-tera/)
- [Código do esp32-tera](https://github.com/fwestarb/esp32-tera)
- [Alpha 1.0, ZIP e SHA-256](https://github.com/fwestarb/esp32-tera/releases/tag/v1.0.0-alpha.1)

O site usa HTML, CSS e JavaScript, sem etapa de compilação ou dependências JavaScript. GitHub Pages publica a pasta `docs` da branch `main`. O arquivo `.nojekyll` mantém os arquivos estáticos. As animações representam a identidade Tera; não são medições da IA. A foto mostra o protótipo real em um teste anterior.

A página tem português/inglês, botões com resposta visual, cópia de comandos, perguntas frequentes, instalação e configuração. O idioma escolhido é salvo apenas no navegador. Não coleta chaves ou configura a placa: essa tarefa pertence ao aplicativo USB e ao portal da própria ESP32.

### Verificações do site

Edge headless com Playwright: português e inglês nas larguras 320, 390, 768 e 1440 px, ausência de rolagem horizontal, tradução completa, persistência do idioma, cópia para a área de transferência, expansão das dúvidas, imagens carregadas e preferência de movimento reduzido. Sem erros de JavaScript. Capturas revisadas visualmente. [Relatório local](TESTE-SITE-ALPHA.json).

Para repetir: sirva `docs` em `http://127.0.0.1:8808` e execute `node tools/test_site.cjs` com Playwright e Microsoft Edge instalados. `TERA_SITE_URL` permite testar outro endereço. As capturas ficam em `private-backups/site-check`, fora do Git.

O ZIP contém firmware, fontes e scripts explicitamente selecionados. Modelos de voz, ambientes Python, gravações, credenciais e dumps de NVS não são distribuídos. SHA-256 acompanha o pacote e o manifesto dos binários. Leia [instalação](INSTALACAO-ALPHA.md) e [resultados do sistema](RESULTADOS-ALPHA-1.0.md) para requisitos e limites.

### Atualizar depois

Mudanças em `docs` publicadas na branch `main` atualizam o site. Para uma nova versão, atualize a versão do firmware, valide o sistema, gere um novo pacote com `tools/package_alpha.py` e publique outra tag/release; ajuste os links de download em `docs/index.html`. Mantenha as versões anteriores disponíveis.

## English

The static site is published from `main` → `/docs` on GitHub Pages. It uses HTML, CSS and JavaScript without a build step. The tree animation is a brand illustration, and the photograph shows a real earlier prototype test. Portuguese/English selection, responsive layout, command copying and reduced-motion support are included. No API keys are collected by the public website.

Playwright checks in headless Edge cover both languages at 320, 390, 768 and 1440 px, overflow, translations, language persistence, clipboard, FAQ expansion, image loading and reduced motion, with no JavaScript errors. Screenshots were visually reviewed. See the linked JSON report and `tools/test_site.cjs` to repeat the checks.

The release includes an explicit selection of source files and firmware, with SHA-256 integrity files. Voice models, environments, recordings, secrets and NVS dumps are excluded. Installation requires Python 3.12 on Windows and a personal provider key. Free APIs have quotas. Firmware/app validation and untested hardware cases are documented separately in the Alpha results.
