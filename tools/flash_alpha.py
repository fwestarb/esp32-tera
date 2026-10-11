"""Flash release partitions only, preserving NVS configuration. Never erases flash."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser();parser.add_argument('--port',required=True);args=parser.parse_args()
    folder=ROOT/'firmware';manifest=json.loads((folder/'manifest.json').read_text())
    files=[]
    for item in manifest['files']:
        target=folder/item['name']
        if hashlib.sha256(target.read_bytes()).hexdigest()!=item['sha256']:raise SystemExit('Arquivo de firmware corrompido.')
        files.extend([item['offset'],str(target)])
    subprocess.run([sys.executable,'-m','esptool','--chip','esp32','--port',args.port,'--baud','460800','write_flash','--flash_mode','dio','--flash_freq','40m','--flash_size','4MB',*files],check=True)
    print('Alpha instalada. A configuracao NVS foi preservada.')
if __name__=='__main__':main()
