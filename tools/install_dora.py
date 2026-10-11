"""Install isolated offline female Portuguese TTS; no API key is required."""
import hashlib
import json
from pathlib import Path
import subprocess
import venv
from urllib.request import urlopen

ROOT=Path(__file__).resolve().parents[1]
BASE='https://github.com/thewh1teagle/kokoro-onnx/releases/download/model-files-v1.1/'

def main():
    environment=ROOT/'.venv-dora';python=environment/'Scripts/python.exe'
    if not python.exists():venv.EnvBuilder(with_pip=True).create(environment)
    subprocess.run([str(python),'-m','pip','install','-r',str(ROOT/'tools/requirements-dora.txt')],check=True)
    folder=ROOT/'models/kokoro';folder.mkdir(parents=True,exist_ok=True)
    expected_file=ROOT/'tools/dora_assets.json'
    expected=json.loads(expected_file.read_text()) if expected_file.exists() else {}
    hashes={}
    for name in ['kokoro-v1.0.onnx','voices-v1.0.bin']:
        target=folder/name
        if not target.exists():
            temporary=target.with_suffix(target.suffix+'.download')
            with urlopen(BASE+name,timeout=60) as response,temporary.open('wb') as output:
                while block:=response.read(1024*1024):output.write(block)
            actual=hashlib.sha256(temporary.read_bytes()).hexdigest()
            if name in expected and expected[name]!=actual:raise ValueError('Hash do modelo incorreto.')
            temporary.replace(target)
        actual=hashlib.sha256(target.read_bytes()).hexdigest()
        if name in expected and expected[name]!=actual:raise ValueError('Modelo local corrompido.')
        hashes[name]=actual
    # Maintainer creates the pinned manifest on first preparation; included in releases.
    if not expected:expected_file.write_text(json.dumps(hashes,indent=2),encoding='utf-8')
    subprocess.run([str(python),str(ROOT/'tools/dora_speech.py'),'--warmup'],check=True)
    print('Dora instalada e validada no PC.')

if __name__=='__main__':main()
