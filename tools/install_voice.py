"""Install optional offline Piper speech locally. No keys, accounts or paid API."""
import hashlib
from pathlib import Path
import subprocess
import sys
from urllib.request import urlopen
import venv

ROOT=Path(__file__).resolve().parents[1]
BASE='https://huggingface.co/rhasspy/piper-voices/resolve/main/pt/pt_BR/faber/medium/'
FILES={
    'pt_BR-faber-medium.onnx':'858555e3a064209c57088fe6bd70c4c3dc54d03eaa00c45d5ecaf43a33f95aa7',
    'pt_BR-faber-medium.onnx.json':'7e694de195ae3fc36dd732c445eb04fb49b649854893cb5506b978f0d50a1d6f',
}

def main():
    environment=ROOT/'.venv-tts'
    python=environment/'Scripts/python.exe'
    if not python.exists():venv.EnvBuilder(with_pip=True).create(environment)
    installed=subprocess.run([str(python),'-c',"import importlib.metadata; assert importlib.metadata.version('piper-tts')=='1.4.2'"],capture_output=True)
    if installed.returncode:
        subprocess.run([str(python),'-m','pip','install','piper-tts==1.4.2'],check=True)
    folder=ROOT/'models/piper';folder.mkdir(parents=True,exist_ok=True)
    for name,expected in FILES.items():
        target=folder/name
        if target.exists() and hashlib.sha256(target.read_bytes()).hexdigest()==expected:continue
        temporary=folder/(name+'.download')
        with urlopen(BASE+name,timeout=60) as response, temporary.open('wb') as output:
            while block:=response.read(512*1024):output.write(block)
        if hashlib.sha256(temporary.read_bytes()).hexdigest()!=expected:
            temporary.unlink();raise ValueError('O modelo baixado nao corresponde ao hash esperado.')
        temporary.replace(target)
    with urlopen(BASE+'MODEL_CARD',timeout=30) as response:
        (folder/'MODEL_CARD').write_bytes(response.read())
    print('Voz Faber instalada. Selecione Faber no aplicativo Tera.')

if __name__=='__main__':main()
