// HTTP/configuration mocked; actual embedded HTML. No board, Wi-Fi or API calls.
const fs = require('node:fs');
const assert = require('node:assert/strict');
const { chromium } = require(process.env.TERA_PLAYWRIGHT || 'playwright');
(async () => {
 const html = fs.readFileSync('data/portal.html','utf8').replaceAll('{{TOKEN}}','a'.repeat(32)).replaceAll('{{SSID}}','Rede teste').replaceAll('{{MODEL}}','openai/gpt-oss-120b').replace('{{GROQ_SELECTED}}','selected').replace('{{OR_SELECTED}}','');
 const browser = await chromium.launch({headless:true,...(process.env.CHROME_PATH?{executablePath:process.env.CHROME_PATH}:{})});
 try {
  const page = await browser.newPage({viewport:{width:390,height:844}}); page.setDefaultTimeout(10000);
  let payload, fail=false;
  await page.route('**/*',route=>{
   if(route.request().url()==='http://tera.test/')return route.fulfill({contentType:'text/html',body:html});
   if(route.request().url()==='http://tera.test/api/config'){
    payload=route.request().postDataJSON();assert.equal(route.request().headers()['x-tera-token'],'a'.repeat(32));
    return route.fulfill({status:fail?400:200,contentType:'application/json',body:JSON.stringify(fail?{error:'Erro sintetico'}:{message:'Salvo sintetico'})});
   }
   return route.abort();
  });
  await page.goto('http://tera.test/'); await page.locator('#password').fill('residual123'); await page.locator('#openNetwork').check();
  assert.equal(await page.locator('#password').inputValue(),''); assert.equal(await page.locator('#password').isDisabled(),true);
  await page.locator('#save').click();await page.locator('#message').filter({hasText:'Salvo sintetico'}).waitFor();
  assert.equal(payload.openNetwork,true);assert.equal(payload.password,'');
  await page.locator('#openNetwork').uncheck();assert.equal(await page.locator('#password').isEnabled(),true);
  await page.locator('#password').fill('secured123');fail=true;await page.locator('#save').click();await page.locator('#message').filter({hasText:'Erro sintetico'}).waitFor();
  assert.equal(payload.password,'secured123');assert.equal(payload.openNetwork,false);assert.equal(await page.locator('#save').isEnabled(),true);
  assert.equal(await page.locator('#password').inputValue(),'secured123');
  fail=false;await page.locator('#save').click();await page.locator('#message').filter({hasText:'Salvo sintetico'}).waitFor();assert.equal(await page.locator('#password').inputValue(),'');
  assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),false);
  console.log('PASS: portal open-network, secured-network, error recovery, secret clearing and mobile layout');
 } finally {await browser.close();}
})().catch(error=>{console.error(error);process.exitCode=1});
