"""Real cloud chat on ESP32 followed by the selected PC neural voice."""
import json
from pathlib import Path
import queue
import time
from pc_bridge import PCBridge

events=queue.Queue();bridge=PCBridge(events.put);report=[]
def wait(predicate,timeout=90):
    end=time.monotonic()+timeout
    while time.monotonic()<end:
        try:event=events.get(timeout=.3)
        except queue.Empty:continue
        safe={k:v for k,v in event.items() if k not in ('session','bridgeSession')};report.append(safe)
        if event.get('event') in ('reply','speech','error'):print(json.dumps(safe,ensure_ascii=False),flush=True)
        if predicate(event):return event
    raise AssertionError('Tempo de resposta esgotado.')
try:
    bridge.connect('COM3');wait(lambda e:e.get('event')=='bridge_ready',20)
    bridge.send({'op':'chat','text':'Responda com uma frase curta confirmando que a Tera esta funcionando.'})
    reply=wait(lambda e:e.get('event') in ('reply','error'));assert reply['event']=='reply',reply
    speech=wait(lambda e:e.get('event')=='speech');assert speech.get('ok') and speech.get('engine')=='piper'
    (Path(__file__).resolve().parents[1]/'docs/TESTE-CHAT-FALA-0.5.1.json').write_text(json.dumps({'date':'2026-10-10','passed':True,'scope':'actual ESP32 Groq chat and real PC Piper playback, not a new live microphone capture','events':report},ensure_ascii=False,indent=2),encoding='utf-8')
finally:bridge.close()
