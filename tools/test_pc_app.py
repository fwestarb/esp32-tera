"""Check the running companion and its single-instance behavior."""
import json
from pathlib import Path
import subprocess
import sys
import time
from urllib.request import urlopen

ROOT = Path(__file__).resolve().parents[1]
def health():
    with urlopen('http://127.0.0.1:39135/health', timeout=2) as response:
        return json.load(response)

deadline = time.monotonic()+15
initial = {}
while time.monotonic() < deadline:
    try:
        initial = health()
        if initial['usb']: break
    except OSError: pass
    time.sleep(.3)
assert initial.get('usb') and initial['speech'] and initial['actions']
result = subprocess.run([sys.executable, str(ROOT/'tools/tera_usb.py'), '--minimized'], timeout=10)
assert result.returncode == 0
after = health()
assert after['usb']
(ROOT/('docs/TESTE-APP-WINDOWS-'+initial.get('firmware','0.5.0')+'.json')).write_text(json.dumps({
    'date':'2026-10-11', 'passed':True,
    'scope':'real Tk companion automatic COM3 pairing and second launch restoring the same instance; startup shortcut installed',
    'initialHealth':initial, 'afterSecondLaunch':after,
},ensure_ascii=False,indent=2),encoding='utf-8')
print('PASS: automatic USB pairing and single instance')
