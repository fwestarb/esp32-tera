"""Physical-board checks and voice monitor. Logs contain no credentials or audio."""
import argparse
import json
import time
from pathlib import Path
import serial

def run():
    parser=argparse.ArgumentParser()
    parser.add_argument('--port',default='COM3')
    parser.add_argument('--op',choices=['check','voice_test','monitor'],default='check')
    parser.add_argument('--seconds',type=int,default=60)
    parser.add_argument('--output',default='docs/TESTE-VOZ-0.4.0.json')
    args=parser.parse_args()
    events=[]
    conn=serial.Serial(port=None,baudrate=115200,timeout=.3)
    conn.dtr=False;conn.rts=False;conn.port=args.port;conn.open()
    with conn:
        time.sleep(2);conn.reset_input_buffer()
        def send(op):conn.write((json.dumps({'op':op})+'\n').encode())
        send('status')
        deadline=time.monotonic()+args.seconds
        checked=False
        requested=False
        next_status=time.monotonic()+3
        while time.monotonic()<deadline:
            if not requested and time.monotonic()>=next_status:
                send('status');next_status=time.monotonic()+3
            raw=conn.readline()
            if not raw:continue
            try:event=json.loads(raw)
            except (ValueError,UnicodeDecodeError):
                line=raw.decode('utf-8',errors='replace').strip()
                if any(marker in line for marker in ('watchdog','Guru Meditation','Debug exception reason','Brownout','Backtrace','rst:',' E (')):
                    events.append({'event':'diagnostic','message':line});print(line,flush=True)
                continue
            events.append(event)
            print(json.dumps(event,ensure_ascii=False),flush=True)
            if event.get('event')=='status':
                checked=True
                if not requested and args.op=='check' and event.get('voiceReady'):
                    send('audio_selftest');requested=True
                elif not requested and args.op=='voice_test' and event.get('voiceReady'):
                    send('voice_test');requested=True
            if args.op=='check' and event.get('event') in ('audio_selftest','error'):break
            if args.op!='check' and event.get('event') in ('reply','error'):break
    Path(args.output).write_text(json.dumps({'date':'2026-10-09','mode':args.op,'target':'physical original TTGO T-Display','events':events},ensure_ascii=False,indent=2),encoding='utf-8')
    if args.op=='check':
        assert any(e.get('event')=='audio_selftest' and e.get('passed') for e in events),'Audio buffer/stream check did not pass'
        assert any(e.get('event')=='audio_selftest' and e.get('tlsWithAudioBuffer') for e in events),'TLS with the audio buffer allocated did not pass'
    if args.op=='voice_test':
        assert any(e.get('event')=='reply' for e in events),'No successful end-to-end voice reply; inspect the report'

if __name__=='__main__':run()
