"""Non-destructive tests against an actual TTGO; never writes valid settings."""
import json, time, serial
from pathlib import Path

def run(port='COM3'):
    results=[]
    conn=serial.Serial(port=None,baudrate=115200,timeout=0.3)
    conn.dtr=False;conn.rts=False;conn.port=port;conn.open()
    with conn:
        time.sleep(2);conn.reset_input_buffer()
        def exchange(payload):
            conn.write((payload+'\n').encode());deadline=time.monotonic()+5
            while time.monotonic()<deadline:
                try:
                    event=json.loads(conn.readline())
                    if event.get('event') in ('status','error','reply','configured'):return event
                except (ValueError,UnicodeDecodeError):pass
            raise AssertionError('No firmware response')
        baseline=exchange('{"op":"status"}')
        baseline.pop('bridgeSession',None)
        assert baseline['version'] in ('0.5.0','0.5.1')
        results.append({'test':'firmware boots and status responds','passed':True,'status':baseline})
        cases=[('invalid JSON','{'),('unknown command','{"op":"unknown"}'),('missing configuration','{"op":"configure"}'),('empty message','{"op":"chat","text":" "}'),('overlong message',json.dumps({'op':'chat','text':'a'*1025})),('oversize serial command','a'*3073),('paid model rejected',json.dumps({'op':'configure','ssid':'test','password':'12345678','key':'sk-or-'+'a'*30,'model':'paid/model'})),('header injection rejected',json.dumps({'op':'configure','ssid':'test','password':'12345678','key':'sk-or-'+'a'*30+'\r\n'}))]
        for name,payload in cases:
            event=exchange(payload);assert event['event']=='error',(name,event)
            results.append({'test':name,'passed':True,'response':event})
        after=exchange('{"op":"status"}')
        assert baseline['configured']==after['configured']
        results.append({'test':'invalid commands do not change configured state','passed':True})
        if not baseline['configured']:
            event=exchange('{"op":"chat","text":"Ola"}');assert event['event']=='error'
            results.append({'test':'no fake AI reply without configuration','passed':True,'response':event})
    report={'date':'2026-10-10','target':'physical TTGO T-Display original','results':results,'scope':'USB protocol validation only; no authenticated AI requests or microphone capture in this suite'}
    Path('docs/TESTES-USB-'+baseline['version']+'.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
    print(json.dumps(report,ensure_ascii=False,indent=2))
if __name__=='__main__':run()
