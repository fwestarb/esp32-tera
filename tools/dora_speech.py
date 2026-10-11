"""Offline Kokoro Dora speech using ONNX CPU, isolated from the USB reader."""
import json
import os
from pathlib import Path
import re
import sys
import tempfile
import time
import winsound
import soundfile as sf
import onnxruntime as ort
from kokoro_onnx import Kokoro

ROOT=Path(__file__).resolve().parents[1]

def main():
    started=time.monotonic()
    options=ort.SessionOptions();options.intra_op_num_threads=min(4,os.cpu_count() or 2)
    session=ort.InferenceSession(str(ROOT/'models/kokoro/kokoro-v1.0.onnx'),sess_options=options,providers=['CPUExecutionProvider'])
    voice=Kokoro.from_session(session,str(ROOT/'models/kokoro/voices-v1.0.bin'))
    if '--warmup' in sys.argv:
        samples,rate=voice.create('Olá, eu sou a Tera.',voice='pf_dora',lang='pt-br')
        assert len(samples)>0
        print(json.dumps({'ok':True,'engine':'kokoro','voice':'Dora','warmup':True}));return
    request=json.load(sys.stdin);text=request.get('text')
    if not isinstance(text,str) or not 1<=len(text)<=4000:raise ValueError('Texto de fala invalido.')
    text=re.sub(r'https?://\S+','link',text)
    text=re.sub(r'[*#`_]','',text)
    speed=request.get('speed',1.0)
    if not isinstance(speed,(int,float)) or not .8<=speed<=1.2:raise ValueError('Velocidade invalida.')
    with tempfile.TemporaryDirectory(prefix='tera-dora-') as temporary:
        output=Path(request.get('wav') or Path(temporary)/'reply.wav')
        samples,rate=voice.create(text,voice='pf_dora',speed=speed,lang='pt-br')
        sf.write(str(output),samples,rate,subtype='PCM_16')
        if not request.get('wav'):winsound.PlaySound(str(output),winsound.SND_FILENAME)
    print(json.dumps({'ok':True,'engine':'kokoro','voice':'Dora','culture':'pt-BR','characters':len(text),'elapsedMs':round((time.monotonic()-started)*1000),'mode':'wav' if request.get('wav') else 'playback'}),flush=True)

if __name__=='__main__':
    try:main()
    except Exception as error:
        print(json.dumps({'ok':False,'engine':'kokoro','error':str(error)[:300]}),flush=True);sys.exit(1)
