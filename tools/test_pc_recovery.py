"""Physical regression: PC actions, neural playback, reset and voice replay."""
import json
import sys
sys.stdout.reconfigure(encoding="utf-8")
from pathlib import Path
import queue
import time
from pc_bridge import PCBridge
from test_pc_device import pcm8

ROOT=Path(__file__).resolve().parents[1]
events=queue.Queue();bridge=PCBridge(events.put);log=[]

def wait(predicate,timeout=90):
    until=time.monotonic()+timeout
    while time.monotonic()<until:
        try:event=events.get(timeout=.3)
        except queue.Empty:continue
        safe={k:v for k,v in event.items() if k not in ('session','bridgeSession','ssid')}
        log.append(safe)
        if event.get('event') not in ('status','bridge_pong'):
            print(json.dumps(safe,ensure_ascii=False),flush=True)
        if predicate(event):return event
    raise AssertionError('Evento esperado nao recebido.')

def check_action():
    bridge.send({'op':'chat','text':'abra a calculadora'})
    action=wait(lambda e:e.get('event')=='action_result' or e.get('event')=='error')
    assert action.get('ok') and action['target']=='calculator',action
    spoken=wait(lambda e:e.get('event')=='speech')
    assert spoken.get('ok') and spoken.get('engine')=='kokoro',spoken

try:
    bridge.connect('COM3');wait(lambda e:e.get('event')=='bridge_ready',20)
    bridge.send({'op':'status'});state=wait(lambda e:e.get('event')=='status')
    assert state['version']=='1.0.0-alpha.1' and state['watchdogSeconds']==120
    first_boot=state['bootId'];assert state['model']=='openai/gpt-oss-120b';check_action()
    bridge.connection.rts=True;time.sleep(.15);bridge.connection.rts=False
    restarted=wait(lambda e:e.get('event')=='status' and e.get('bootId')!=first_boot,30)
    wait(lambda e:e.get('event')=='bridge_ready',20)
    assert restarted['model']=='openai/gpt-oss-120b'
    check_action()
    bridge.send({'op':'status'});wait(lambda e:e.get('event')=='status' and e.get('voiceReady'),60)
    assert bridge.send_fixture(pcm8(ROOT/'recordings/tera-comando-calculadora.wav'))
    transcript=wait(lambda e:e.get('event') in ('transcript','error'))
    assert transcript['event']=='transcript' and 'calculadora' in transcript['message'].lower(),transcript
    action=wait(lambda e:e.get('event')=='action_result' or e.get('event')=='error')
    assert action.get('ok'),action
    spoken=wait(lambda e:e.get('event')=='speech');assert spoken.get('ok') and spoken.get('engine')=='kokoro'
    # Continue receiving real heartbeats rather than trusting an old "ready" flag.
    end=time.monotonic()+16
    while time.monotonic()<end:wait(lambda e:e.get('event') in ('status','bridge_pong'),10)
    assert bridge.responsive()
    summary={'date':'2026-10-11','passed':True,'version':'1.0.0-alpha.1','voice':'Kokoro Dora pt-BR',
             'scope':'physical USB actions and real playback before/after ESP32 reset; Groq STT of synthetic voice command; 16s idle heartbeat check. No new live microphone recording.',
             'events':log}
    (ROOT/'docs/TESTE-RECUPERACAO-1.0.0-alpha.1.json').write_text(json.dumps(summary,ensure_ascii=False,indent=2),encoding='utf-8')
    print('PASS: actions, neural speech, reset recovery and cloud voice command')
finally:bridge.close()
