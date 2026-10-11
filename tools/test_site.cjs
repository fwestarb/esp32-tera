// Public documentation checks. Run with Playwright available in NODE_PATH.
const { chromium } = require('playwright');
const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
(async () => {
  const url = process.env.TERA_SITE_URL || 'http://127.0.0.1:8808';
  const output = path.resolve(__dirname, '../private-backups/site-check');
  fs.mkdirSync(output, { recursive: true });
  const browser = await chromium.launch({ channel: 'msedge', headless: true });
  const context = await browser.newContext({ permissions: ['clipboard-read', 'clipboard-write'] });
  const page = await context.newPage();
  const errors = [];
  page.on('pageerror', error => errors.push(error.message));
  const response = await page.goto(url);
  assert.equal(response.status(), 200);
  await page.locator('h1').waitFor();
  const original = await page.locator('[data-i18n]').evaluateAll(nodes => nodes.map(el => ({ key: el.dataset.i18n, text: el.innerHTML })));
  await page.locator('[data-lang="en"]').click();
  assert.equal(await page.locator('html').getAttribute('lang'), 'en');
  assert.match(await page.locator('h1').innerText(), /A small board/);
  const english = await page.locator('[data-i18n]').evaluateAll(nodes => nodes.map(el => ({ key: el.dataset.i18n, text: el.innerHTML })));
  const exceptions = new Set(['pythonLink', 'commandExample']);
  assert.equal(english.filter((row, index) => row.text === original[index].text && !exceptions.has(row.key)).length, 0, 'Missing English translation');
  await page.reload();
  assert.equal(await page.locator('html').getAttribute('lang'), 'en', 'Language persistence');
  const widths = [320, 390, 768, 1440];
  const layouts = [];
  for (const language of ['pt', 'en']) {
    await page.locator(`[data-lang="${language}"]`).click();
    for (const width of widths) {
      await page.setViewportSize({ width, height: 1000 });
      assert.equal(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth + 1), true, `${language} ${width}: horizontal overflow`);
      layouts.push({ language, width, overflow: false });
    }
  }
  await page.locator('[data-lang="pt"]').click();
  await page.locator('[data-copy="instalar.cmd"]').click();
  assert.equal(await page.evaluate(() => navigator.clipboard.readText()), 'instalar.cmd');
  assert.equal(await page.locator('#toast').innerText(), 'Comando copiado');
  await page.waitForTimeout(250);
  assert.equal(await page.locator('#toast').evaluate(el => getComputedStyle(el).opacity), '1');
  await page.locator('#duvidas').scrollIntoViewIfNeeded();
  await page.locator('summary').first().click();
  assert.equal(await page.locator('details').first().getAttribute('open'), '');
  for (const element of await page.locator('.reveal').all()) { await element.scrollIntoViewIfNeeded(); }
  await page.waitForTimeout(900);
  assert.equal(await page.locator('.reveal').evaluateAll(nodes => nodes.every(el => getComputedStyle(el).opacity === '1')), true);
  assert.equal(await page.locator('img').evaluateAll(nodes => nodes.every(el => el.complete && el.naturalWidth > 0)), true);
  await page.screenshot({ path: path.join(output, 'desktop.png'), fullPage: true });
  await page.setViewportSize({ width: 390, height: 844 });
  await page.screenshot({ path: path.join(output, 'mobile.png'), fullPage: true });
  await page.emulateMedia({ reducedMotion: 'reduce' });
  assert.equal(await page.locator('.trunk').evaluate(el => getComputedStyle(el).strokeDashoffset), '0px');
  assert.equal(await page.locator('.leaves ellipse').evaluateAll(nodes => nodes.every(el => getComputedStyle(el).opacity === '1')), true);
  assert.deepEqual(errors, []);
  const report = { url, http: 200, layouts, translation: 'complete', languagePersistence: true, clipboard: true, faq: true, images: true, reducedMotion: true, javascriptErrors: errors };
  fs.writeFileSync(path.join(output, 'report.json'), JSON.stringify(report, null, 2));
  console.log(JSON.stringify(report, null, 2));
  await browser.close();
})().catch(error => { console.error(error); process.exitCode = 1; });
