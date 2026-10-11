const fs=require('fs');const path=require('path');
const {chromium}=require(process.env.TERA_PLAYWRIGHT || 'playwright');
(async()=>{
 let html=fs.readFileSync('data/portal.html','utf8').replaceAll('{{TOKEN}}','a'.repeat(32)).replaceAll('{{SSID}}','Minha rede').replaceAll('{{MODEL}}','openai/gpt-oss-120b').replace('{{GROQ_SELECTED}}','selected').replace('{{OR_SELECTED}}','');
 const browser=await chromium.launch({channel:'msedge',headless:true});
 const page=await browser.newPage({viewport:{width:390,height:844}});let payload=null;let invalid=false;const errors=[];
 page.on('pageerror',e=>errors.push(e.message));
 await page.route('http://tera.test/**',async route=>{
  if(route.request().url().endsWith('/api/config')){
   payload=route.request().postDataJSON();
   if(route.request().headers()['x-tera-token']!=='a'.repeat(32))throw Error('missing token');
   await route.fulfill({status:invalid?400:200,contentType:'application/json',body:JSON.stringify(invalid?{error:'Confira a senha.'}:{ok:true,message:'Salvo. A rede Tera sera fechada.'})});
  }else await route.fulfill({contentType:'text/html',body:html});
 });
 await page.goto('http://tera.test/');
 await page.locator('#password').fill('12345678');await page.locator('#key').fill('gsk_test_not_a_real_key');
 await page.locator('#save').click();await page.waitForFunction(()=>document.querySelector('#message').textContent.startsWith('Salvo.'));
 if(payload.model!=='openai/gpt-oss-120b')throw Error('wrong default model');
 if(payload.ssid!=='Minha rede'||payload.provider!=='groq'||payload.password!=='12345678')throw Error('wrong form data');
 if(await page.locator('#key').inputValue()||await page.locator('#password').inputValue())throw Error('secrets not cleared');
 invalid=true;await page.locator('#save').click();await page.waitForFunction(()=>document.querySelector('#message').textContent==='Confira a senha.');
 if(!await page.locator('#save').isEnabled())throw Error('button remains disabled');
 await page.locator('#provider').selectOption('openrouter');if(!((await page.locator('#key').getAttribute('placeholder')).startsWith('sk-or-')))throw Error('provider placeholder');
 if(await page.evaluate(()=>document.documentElement.scrollWidth>window.innerWidth))throw Error('horizontal overflow');
 if(await page.locator('#model').inputValue()!=='openrouter/free')throw Error('provider model mismatch');
 await page.locator('#modelSelect').selectOption('custom');await page.locator('#model').fill('test/model:free');invalid=false;await page.locator('#save').click();await page.waitForFunction(()=>document.querySelector('#message').textContent.startsWith('Salvo.'));if(payload.model!=='test/model:free')throw Error('custom model not submitted');
 await page.locator('#provider').selectOption('groq');await page.locator('#modelSelect').selectOption('openai/gpt-oss-20b');await page.locator('#save').click();await page.waitForFunction(()=>document.querySelector('#message').textContent.startsWith('Salvo.'));if(payload.model!=='openai/gpt-oss-20b')throw Error('model switch');await page.locator('#message').evaluate(e=>e.textContent='');
 fs.mkdirSync('docs/images',{recursive:true});await page.screenshot({path:'docs/images/configuracao-celular.png',fullPage:true});
 if(errors.length)throw Error(errors.join(';'));
 fs.writeFileSync('docs/TESTE-INTERFACE-PORTAL-1.0.0-alpha.1.json',JSON.stringify({date:'2026-10-11',scope:'actual embedded HTML in headless Edge, 390px mobile viewport; HTTP mocked, no actual phone',passed:true,checks:['form payload and anti-CSRF header','clears secret fields after saving','shows backend validation error','re-enables save after failure','provider placeholder','no horizontal overflow','no JavaScript exceptions','default model submitted','OpenRouter free model and custom :free submitted','Groq model switch submitted']},null,2));
 await browser.close();console.log('PASS: mobile portal UI (10 checks)');
})().catch(e=>{console.error(e);process.exit(1)});
