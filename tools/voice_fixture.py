"""Replay a previously captured test WAV through the physical ESP32's cloud flow.

This checks transcription/chat integration, not live ADC capture or button wiring.
The key stays on the device. The ESP32 uploads the PCM8 WAV itself.
"""
import argparse
from array import array
import hashlib
import json
from pathlib import Path
import time
import wave
import zlib
import serial

parser=argparse.ArgumentParser()
parser.add_argument('wav',type=Path)
parser.add_argument('--port',default='COM3')
parser.add_argument('--output',type=Path,default=Path('docs/TESTE-VOZ-REPLAY-0.4.0.json'))
args=parser.parse_args()
with wave.open(str(args.wav),'rb') as wav:
    assert (wav.getnchannels(),wav.getsampwidth(),wav.getframerate())==(1,2,8000)
    samples=array('h',wav.readframes(wav.getnframes()))
assert 4000<=len(samples)<=80000
mean=sum(samples)/len(samples)
peak=max(abs(s-mean) for s in samples)
gain=114/peak if peak else 1
payload=bytes(max(0,min(255,round((s-mean)*gain)+128)) for s in samples)
events=[]
conn=serial.Serial(port=None,baudrate=115200,timeout=.3,write_timeout=20)
conn.dtr=False;conn.rts=False;conn.port=args.port;conn.open()
with conn:
    time.sleep(1);conn.reset_input_buffer()
    def send(data):conn.write((json.dumps(data)+'\n').encode())
    send({'op':'status'})
    deadline=time.monotonic()+120
    next_status=time.monotonic()+3
    requested=False
    while time.monotonic()<deadline:
        if not requested and time.monotonic()>=next_status:
            send({'op':'status'});next_status=time.monotonic()+3
        raw=conn.readline()
        if not raw:continue
        try:event=json.loads(raw)
        except (ValueError,UnicodeDecodeError):
            line=raw.decode('utf-8',errors='replace').strip()
            if any(marker in line for marker in ('watchdog','Guru Meditation','Debug exception reason','Brownout','Backtrace','rst:',' E (')):
                event={'event':'diagnostic','message':line}
                events.append(event);print(json.dumps(event,ensure_ascii=False),flush=True)
            continue
        events.append(event);print(json.dumps(event,ensure_ascii=False),flush=True)
        kind=event.get('event')
        if requested and kind=='status' and not event.get('wifi'):
            events.append({'event':'error','message':'Device restarted during the request'});break
        if kind=='status' and event.get('voiceReady') and not requested:
            send({'op':'probe_groq_auth'})
            requested=True
        elif kind=='network' and 'autenticacao Groq' in event.get('message',''):
            if not event['message'].endswith('HTTP 200'):break
            send({'op':'voice_fixture','samples':len(payload),'crc':zlib.crc32(payload)})
        elif kind=='fixture_ready':
            assert conn.write(payload)==len(payload)
        elif kind in ('reply','error'):break
args.output.write_text(json.dumps({'date':'2026-10-09','mode':'prerecorded audio replayed through physical TTGO; NOT live/button test','source':args.wav.name,'sourceSha256':hashlib.sha256(args.wav.read_bytes()).hexdigest(),'samples':len(payload),'events':events},ensure_ascii=False,indent=2),encoding='utf-8')
assert any(e.get('event')=='transcript' for e in events),'No successful transcription'
assert any(e.get('event')=='reply' for e in events),'No successful chat reply'
