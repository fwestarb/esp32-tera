"""Prepare a local release ZIP from an explicit source allowlist, never NVS."""
import hashlib
import json
from pathlib import Path
import shutil
import zipfile

ROOT=Path(__file__).resolve().parents[1]
VERSION='1.0.0-alpha.1'
def main():
    build=ROOT/'.pio/build/ttgo_t_display';folder=ROOT/'dist'/('esp32-tera-'+VERSION)
    folder.mkdir(parents=True,exist_ok=True);firmware=folder/'firmware';firmware.mkdir(exist_ok=True)
    rows=[]
    for offset,name,source in [('0x1000','bootloader.bin',build/'bootloader.bin'),('0x8000','partitions.bin',build/'partitions.bin'),('0xe000','boot_app0.bin',Path.home()/'.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin'),('0x10000','firmware.bin',build/'firmware.bin')]:
        shutil.copyfile(source,firmware/name);rows.append({'offset':offset,'name':name,'sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'bytes':source.stat().st_size})
    (firmware/'manifest.json').write_text(json.dumps({'version':VERSION,'board':'original TTGO T-Display','files':rows},indent=2),encoding='utf-8')
    names=['README.md','LICENSE','configurar.cmd','instalar.cmd','gravar-firmware.cmd','platformio.ini','config.example.json']
    for directory in ['include','src','data']:
        names.extend(str(p.relative_to(ROOT)) for p in (ROOT/directory).rglob('*') if p.is_file())
    for name in ['tera_usb.py','pc_bridge.py','speech.ps1','piper_speech.py','dora_speech.py','install_pc.py','install_voice.py','install_dora.py','dora_assets.json','flash_alpha.py','requirements.txt','requirements-dora.txt','install_pc_startup.ps1','test_pc_bridge.py','test_device.py','test_alpha_device.py','test_pc_recovery.py','test_pc_device.py','test_pc_app.py','test_portal_ui.cjs','package_alpha.py']:
        names.append('tools/'+name)
    for name in ['INSTALACAO-ALPHA.md','THIRD-PARTY.md','RESULTADOS-ALPHA-1.0.md','HARDWARE.md','PUBLICACAO.md','TESTE-SITE-ALPHA.json']:
        names.append('docs/'+name)
    names.append('docs/images/configuracao-celular.png')
    for source in (ROOT/'docs').glob('*ALPHA*.json'):
        names.append(str(source.relative_to(ROOT)))
    for name in ['TESTES-USB-1.0.0-alpha.1.json','TESTE-INTERFACE-PORTAL-1.0.0-alpha.1.json','TESTE-RECUPERACAO-1.0.0-alpha.1.json','TESTE-APP-WINDOWS-1.0.0-alpha.1.json']:
        names.append('docs/'+name)
    names=sorted(set(name.replace('\\','/') for name in names))
    for name in names:
        source=ROOT/name;target=folder/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(source,target)
    archive=Path(str(folder)+'.zip')
    with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED) as zipped:
        # Never include leftovers or files someone placed in the staging folder.
        for name in sorted(set(names+['firmware/manifest.json']+['firmware/'+row['name'] for row in rows])):
            zipped.write(folder/name,name)
    (ROOT/'dist/SHA256SUMS.txt').write_text(hashlib.sha256(archive.read_bytes()).hexdigest()+'  '+archive.name+'\n',encoding='utf-8')
    print(json.dumps({'archive':str(archive),'bytes':archive.stat().st_size,'files':len(names)+5,'firmware':rows},indent=2))
if __name__=='__main__':main()
