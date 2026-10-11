"""Alpha integration against the physical ESP32. Keys never leave the board."""
import json
import sys
sys.stdout.reconfigure(encoding="utf-8")
from pathlib import Path
import queue
import time
from pc_bridge import PCBridge

ROOT=Path(__file__).resolve().parents[1]
CASES=[
 ('Abra a calculadora','calculator'),('pode abrir a calculadora?','calculator'),
 ('Tera, abre a calculadora, por favor!','calculator'),('Você poderia abrir o Bloco de notas para mim?','notepad'),
 ('Por favor abra o explorador de arquivos','explorer'),('quero que você abra o navegador','browser'),
 ('consegue abrir o Git Hub?','github'),('gostaria de abrir o YouTube','youtube'),
 ('Pode abrir a calcula dora','calculator'),('Não abra a calculadora',''),
 ('não, abra a calculadora',''),('Nunca abra o navegador',''),('Como abrir a calculadora?',''),
 ('Se eu pedir, abra a calculadora',''),('Diga "abra a calculadora"',''),
 ('“abra a calculadora”',''),('Pode abrir algo?','clarify'),
 ('Abra a calculadora e o navegador','clarify'),('abra powershell','clarify'),
 ('abra calc.exe /c comando','clarify'),('Oi Tera',''),
]

def run():
 events=queue.Queue();bridge=PCBridge(events.put);bridge.speech.voice='kokoro-dora';results=[]
 def wait(kind,timeout=90):
  end=time.monotonic()+timeout
  while time.monotonic()<end:
   try:event=events.get(timeout=.2)
   except queue.Empty:continue
   if event.get('event')=='error':raise AssertionError(event.get('message'))
   if event.get('event')==kind:return event
  raise AssertionError('Timeout '+kind)
 def clean():
  while not events.empty():events.get_nowait()
 def status():
  clean();bridge.send({'op':'status'});return wait('status',20)
 try:
  bridge.connect('COM3');wait('bridge_ready',20)
  initial=status();assert initial['version']=='1.0.0-alpha.1'
  deadline=time.monotonic()+60
  while not initial['voiceReady'] and time.monotonic()<deadline:
   time.sleep(1);initial=status()
  assert initial['voiceReady'],'Aguarde Wi-Fi e relógio HTTPS'
  for text,target in CASES:
   clean();bridge.send({'op':'intent_test','text':text});event=wait('intent_test',10)
   assert event['target']==target,(text,event)
   results.append({'test':'intent','input':text,'target':target,'passed':True})
  # Model-only writes do not require retrieving or resending any secret.
  for model in ['openai/gpt-oss-20b','openai/gpt-oss-120b']:
   clean();bridge.send({'op':'set_model','model':model});wait('configured');assert status()['model']==model
   started=time.monotonic();bridge.send({'op':'chat','text':'Diga seu nome e em qual dispositivo está funcionando. Responda em uma frase.'})
   reply=wait('reply');spoken=wait('speech');assert spoken.get('ok') and spoken.get('engine')=='kokoro'
   assert 'tera' in reply['message'].lower() and any(w in reply['message'].lower() for w in ['ttgo','esp32','t-display'])
   results.append({'test':'actual cloud model + context + female PC playback','model':model,'reply':reply['message'],'speech':spoken,'elapsedSeconds':round(time.monotonic()-started,2),'passed':True})
  clean();bridge.send({'op':'chat','text':'Pode abrir a calculadora, por favor?'})
  action=wait('action_result');assert action.get('ok') and action['target']=='calculator'
  reply=wait('reply');spoken=wait('speech');assert spoken.get('ok')
  results.append({'test':'polite command actual Windows calculator and Dora playback','passed':True,'reply':reply['message'],'speech':spoken})
  clean();bridge.send({'op':'chat','text':'Pode abrir alguma coisa?'})
  reply=wait('reply');assert 'Qual aplicativo' in reply['message'];wait('speech')
  results.append({'test':'ambiguous command asks clarification','passed':True})
  clean();bridge.send({'op':'portal_test'});report=wait('portal_test',45);assert report['passed'],report
  results.append({'test':'actual board HTTP portal, DNS, validation, credential preservation','passed':True,'report':report})
  time.sleep(4);bridge.send({'op':'status'});state=wait('status',20)
  if state.get('portal'):bridge.send({'op':'portal','enabled':False});wait('network')
  bridge.send({'op':'clear_history'});wait('network')
  deadline=time.monotonic()+30
  final=status()
  while not final['wifi'] and time.monotonic()<deadline:
   time.sleep(1);final=status()
  assert final['wifi'] and final['voiceReady']
  results.append({'test':'final state','passed':True,'status':{k:v for k,v in final.items() if k not in ('ssid','bridgeSession','portalName')}})
 finally:bridge.close()
 report={'date':'2026-10-11','passed':True,'scope':'physical ESP32, authenticated cloud chat, real PC application and audio playback; no new human recording or actual phone','results':results}
 (ROOT/'docs/TESTES-ALPHA-1.0.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
 print(json.dumps(report,ensure_ascii=False,indent=2),flush=True)

if __name__=='__main__':run()
