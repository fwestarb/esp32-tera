# Site esp32-tera

## Português

React + Vite. A página publicada continua sendo estática no GitHub Pages; não precisa de servidor ou banco de dados.

```text
cd website
npm ci
npm run build
```

O build gera `docs/ui/assets` e atualiza `docs/index.html`. A pasta `docs` continua contendo a documentação do firmware. Para ver a versão final localmente, sirva `docs` por HTTP a partir da raiz do projeto. `npm run dev` permite editar com atualização automática; as imagens e o ZIP são servidos da pasta `docs`.

`src/content.js` contém os textos PT/EN. `src/App.jsx` lista as imagens disponíveis. Substitua/adicione apenas fotografias e demonstrações reais com legendas precisas; a árvore é uma ilustração. As próximas fotos enviadas pelo autor podem ser adicionadas à galeria sem alterar o firmware.

### Componentes enviados pelo autor

| Componente | Uso |
|---|---|
| FlexCarousel | Galeria, arraste, setas e ampliação; rolagem vertical preservada |
| GlideSelect | Idioma PT/EN, teclado e fechamento por Escape |
| LatticeLoader | Transferência real do ZIP, cancelamento, erro e conclusão |
| HalftoneReveal | Efeito de pontos sobre a árvore ao mover o ponteiro |
| FoldText | Entrada do título objetivo |
| SideRays | Luz discreta na abertura e seção de uso, sobre base sólida |

Os arquivos foram integrados a partir dos seis anexos, com adaptações para cores Tera, transparência PNG, movimento reduzido, acessibilidade e rolagem da página. Licença React Bits preservada em `src/components/LICENSE-REACT-BITS.md`; os componentes não são oferecidos como biblioteca independente. O texto e a lógica próprios do projeto usam MIT. [Avisos de terceiros](../docs/THIRD-PARTY.md).

### Dappled Light — pendente

Não instalado: o autor informou não possuir licença Pro. `components.json` tem os registries oficiais com referência à variável `REACTBITS_LICENSE_KEY`, sem chave real. Para integrar no futuro, será necessário ter licença, configurar a chave apenas no ambiente local e concluir a configuração shadcn/Tailwind conforme a documentação oficial antes de executar `npx shadcn@latest add @reactbits-starter/dappled-light-tw`. Nenhuma compra ou acesso autenticado foi realizado.

### Download

`docs/downloads/esp32-tera-1.0.0-alpha.1.zip` é uma cópia idêntica do arquivo da release, para permitir transferência na mesma origem. O navegador mostra o LatticeLoader enquanto recebe os bytes; o SHA-256 é validado antes da entrega ao gerenciador de downloads. A interface não afirma que o usuário salvou o arquivo em disco. Falhas têm link para a release. Ao atualizar a versão, altere o nome e hash em `src/content.js` e o ZIP espelhado.

### Testes

Depois do build, sirva `docs` em `http://127.0.0.1:8808/`. Com Playwright e Edge instalados, execute `node tools/test_site.cjs` a partir da raiz do projeto. Verifica layout, seletor, área de transferência, galeria e download real; injeta respostas corrompidas e falhas de rede para verificar os erros. `TERA_SITE_URL` seleciona o endereço publicado.

## English

Build with `npm ci` and `npm run build` in `website`. Vite emits assets into `docs/ui` and updates the Pages entry in `docs/index.html`. Serve `docs` over HTTP to preview the complete production site. Copy and assets are maintained in `src/content.js` and `src/App.jsx`.

Six supplied React Bits components power the gallery, language selector, download status, tree reveal, title entrance and subtle light effects. Reduced-motion and no-WebGL modes use static images. Their license is preserved separately from original MIT project code. Dappled Light remains uninstalled because no Pro license is available; registry configuration contains only an environment-variable placeholder.

The same-origin ZIP matches the GitHub release byte for byte. Streaming progress and SHA-256 validation happen before handing the file to the browser. Cancellation, network errors and integrity errors are handled; saving the file to disk is controlled by the browser. Test coverage is documented in the site report.
