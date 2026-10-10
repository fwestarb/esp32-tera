"""Autonomous physical integration: recordings -> ESP32 -> Groq -> PC speech/action.
No new live microphone capture and no user participation. Keys remain on ESP32.
"""
from array import array
import argparse
import audioop
import hashlib
import json
from pathlib import Path
import queue
import time
import wave
from pc_bridge import PCBridge

ROOT=Path(__file__).resolve().parents[1]
def pcm8(path):
 with wave.open(str(path),'rb') as wav:
  assert wav.getnchannels()==1 and wav.getsampwidth()==2
  raw=wav.readframes(wav.getnframes());rate=wav.getframerate()
 if rate!=8000:raw=audioop.ratecv(raw,2,1,rate,8000,None)[0]
 samples=array('h',raw);mean=sum(samples)/len(samples);peak=max(abs(s-mean) for s in samples)
 assert 4000<=len(samples)<=80000 and peak>0
 return bytes(max(0,min(255,round((s-mean)*114/peak)+128)) for s in samples)

def run():
 parser=argparse.ArgumentParser();parser.add_argument('--port',default='COM3');args=parser.parse_args()
 q=queue.Queue();bridge=PCBridge(q.put);results=[]
 def sanitized(event):return {k:v for k,v in event.items() if k not in ('session','portalName')}
 def wait(predicate,timeout=120):
  end=time.monotonic()+timeout
  while time.monotonic()<end:
   try:event=q.get(timeout=.3)
   except queue.Empty:continue
   if event.get('event') not in ('status','bridge_ready','usb'):print(json.dumps(sanitized(event),ensure_ascii=False),flush=True)
   if predicate(event):return event
  raise AssertionError('Integration event timed out')
 try:
  bridge.connect(args.port);wait(lambda e:e.get('event')=='bridge_ready',20)
  bridge.send({'op':'status'});state=wait(lambda e:e.get('event')=='status' and e.get('voiceReady'),60)
  results.append({'test':'USB pairing and cloud ready','passed':True,'status':sanitized(state)})
  # Validated original physical microphone recording, followed by synthesized command.
  fixtures=[('previous microphone recording',ROOT/'recordings/microfone-2026-10-09-10seg-capacitor-teste2.wav',False),('synthesized voice command',ROOT/'recordings/tera-comando-calculadora.wav',True)]
  for label,path,action in fixtures:
   while not q.empty():q.get_nowait()
   started=time.monotonic();assert bridge.send_fixture(pcm8(path))
   events=[];end=time.monotonic()+150;spoken=False
   while time.monotonic()<end:
    event=wait(lambda e:True,max(.5,end-time.monotonic()));events.append(sanitized(event))
    if event.get('event')=='error':raise AssertionError(event['message'])
    if event.get('event')=='speech':spoken=bool(event.get('ok'));break
   assert spoken,'No completed PC audio playback'
   assert any(e.get('event')=='transcript' for e in events)
   assert any(e.get('event')=='reply' for e in events)
   assert any(e.get('event')=='audio_upload' and e.get('http')==200 for e in events)
   if action:assert any(e.get('event')=='action_result' and e.get('ok') and e.get('target')=='calculator' for e in events)
   results.append({'test':label,'passed':True,'source':path.name,'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'elapsedSeconds':round(time.monotonic()-started,2),'events':events})
  bridge.send({'op':'chat','text':'Meu nome para este teste e Felipe. Responda apenas entendido.'})
  first=wait(lambda e:e.get('event') in ('reply','error'));assert first['event']=='reply',first
  wait(lambda e:e.get('event')=='speech')
  bridge.send({'op':'chat','text':'Qual nome informei na mensagem anterior? Responda apenas o nome.'})
  second=wait(lambda e:e.get('event') in ('reply','error'));assert second['event']=='reply' and 'Felipe' in second['message'],second
  spoken=wait(lambda e:e.get('event')=='speech');assert spoken.get('ok')
  results.append({'test':'bounded last-turn context','passed':True,'firstReply':first['message'],'secondReply':second['message'],'speech':spoken})
  bridge.send({'op':'clear_history'});wait(lambda e:e.get('event')=='network')
  bridge.send({'op':'status'});final=wait(lambda e:e.get('event')=='status')
  assert not final['history']
  results.append({'test':'conversation reset and final state','passed':True,'status':sanitized(final)})
 finally:bridge.close()
 report={'date':'2026-10-10','scope':'physical ESP32 cloud requests + Windows real playback and calculator launch; replay/synthetic audio, not new live ADC/button recording','results':results}
 (ROOT/'docs/TESTE-INTEGRACAO-PC-0.5.0.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
 print('PASS: autonomous physical integration',flush=True)

if __name__=='__main__':run()
