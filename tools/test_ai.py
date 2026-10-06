"""Manual opt-in real API test using credentials already on the board."""
import json,time,serial
from pathlib import Path
results=[]
with serial.Serial('COM3',115200,timeout=.5) as s:
 s.dtr=False;s.rts=False;time.sleep(2);s.reset_input_buffer()
 ready=False
 until=time.monotonic()+60
 while time.monotonic()<until:
  s.write(b'{"op":"status"}\n')
  wait=time.monotonic()+3
  while time.monotonic()<wait:
   try:
    state=json.loads(s.readline())
    if state.get('event')=='status' and state.get('wifi'):ready=True;break
   except ValueError:pass
  if ready:break
 if not ready:raise RuntimeError('WiFi did not connect within 60 seconds')
 time.sleep(5)
 s.write(b'{"op":"chat","text":"Responda apenas: teste OK"}\n')
 deadline=time.monotonic()+60
 while time.monotonic()<deadline:
  try:
   event=json.loads(s.readline());results.append(event);print(json.dumps(event,ensure_ascii=False),flush=True)
   if event.get('event') in ('reply','error'):break
  except ValueError:pass
 else:raise RuntimeError('Timeout waiting for board')
Path('docs/TESTE-IA-REAL.json').write_text(json.dumps({'firmware':'0.2.2','prompt':'Responda apenas: teste OK','events':results},ensure_ascii=False,indent=2),encoding='utf-8')
