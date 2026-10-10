"""Local Portuguese neural speech. Runs in the isolated optional Piper environment."""
import json
from pathlib import Path
import sys
import tempfile
import time
import wave
import winsound
from piper import PiperVoice, SynthesisConfig

ROOT = Path(__file__).resolve().parents[1]

def main():
    request = json.load(sys.stdin)
    text = request.get('text')
    if not isinstance(text, str) or not 1 <= len(text) <= 4000:
        raise ValueError('Texto de fala invalido.')
    started = time.monotonic()
    voice = PiperVoice.load(str(ROOT/'models/piper/pt_BR-faber-medium.onnx'))
    config = SynthesisConfig(length_scale=1.0, volume=0.8)
    with tempfile.TemporaryDirectory(prefix='tera-speech-') as temporary:
        output = Path(request.get('wav') or Path(temporary)/'reply.wav')
        with wave.open(str(output), 'wb') as wav:
            voice.synthesize_wav(text, wav, syn_config=config)
        if not request.get('wav'):
            winsound.PlaySound(str(output), winsound.SND_FILENAME)
    print(json.dumps({'ok':True,'engine':'piper','voice':'Faber','culture':'pt-BR',
                      'characters':len(text),'elapsedMs':round((time.monotonic()-started)*1000),
                      'mode':'wav' if request.get('wav') else 'playback'}), flush=True)

if __name__ == '__main__':
    try: main()
    except Exception as error:
        print(json.dumps({'ok':False,'engine':'piper','error':str(error)[:300]}), flush=True)
        sys.exit(1)
