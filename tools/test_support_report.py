import json
from pathlib import Path
import tempfile
import unittest
import ast
import contextlib
import io
import shutil
import subprocess
import zipfile
from types import SimpleNamespace
from unittest.mock import Mock, patch
from support_report import build_report, export_report

class SupportTests(unittest.TestCase):
    def bridge(self):
        return SimpleNamespace(connection=SimpleNamespace(is_open=True),ready=True,busy=False,actions=True,speech=SimpleNamespace(enabled=False),last_status={'version':'1.0.0-alpha.2','wifi':True,'key':'CANARY','ssid':'CANARY','bridgeSession':'CANARY','message':'CANARY','nested':{'token':'CANARY'}},session='CANARY',last_error='CANARY',current_port='CANARY',send=Mock())
    def test_exports_roundtrip_without_sensitive_fields_or_mutation(self):
        b=self.bridge();before=b.last_status.copy()
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'diagnóstico.json';report=export_report(b,path)
            self.assertEqual(json.loads(path.read_text(encoding='utf-8')),report)
        self.assertEqual(report['schemaVersion'],1)
        self.assertEqual(report['device'],{'firmware':'1.0.0-alpha.2','wifiConnected':True})
        self.assertEqual(report['companion'],{'connected':True,'ready':True,'busy':False,'actionsEnabled':True,'speechEnabled':False})
        self.assertNotIn('CANARY',json.dumps(report));self.assertEqual(b.last_status,before);b.send.assert_not_called()
    def test_disconnected_is_exportable(self):
        b=self.bridge();b.connection=None;b.ready=False;b.last_status={}
        r=build_report(b);self.assertFalse(r['companion']['connected']);self.assertFalse(r['companion']['ready']);self.assertIsNone(r['device']['firmware'])
    def test_raw_values_cannot_leak_through_allowed_keys(self):
        for version in ['CANARY',{'key':'CANARY'},None,123]:
            b=self.bridge();b.last_status={'version':version,'wifi':'CANARY'}
            self.assertEqual(build_report(b)['device'],{'firmware':None,'wifiConnected':None})
    def test_write_failure_is_reported_to_caller(self):
        with tempfile.TemporaryDirectory() as d:
            with self.assertRaises(OSError):export_report(self.bridge(),Path(d)/'missing'/'report.json')
    def test_dialog_callback_cancel_success_and_write_failure(self):
        # Execute the actual nested callback without starting USB, IPC or Tk.
        source=Path(__file__).with_name('tera_usb.py').read_text(encoding='utf-8')
        function=next(n for n in ast.walk(ast.parse(source)) if isinstance(n,ast.FunctionDef) and n.name=='save_diagnostics')
        for destination,error in [('',None),('example.json',None),('example.json',OSError('fixture'))]:
            dialog=SimpleNamespace(asksaveasfilename=Mock(return_value=destination))
            messages=SimpleNamespace(showinfo=Mock(),showerror=Mock())
            exporter=Mock(side_effect=error);bridge=self.bridge()
            namespace=dict(filedialog=dialog,messagebox=messages,root=object(),bridge=bridge,export_report=exporter)
            exec(compile(ast.Module(body=[function],type_ignores=[]),'tera_usb.py','exec'),namespace)
            namespace['save_diagnostics']()
            self.assertEqual(exporter.call_count,int(bool(destination)))
            self.assertEqual(messages.showinfo.call_count,int(bool(destination) and error is None))
            self.assertEqual(messages.showerror.call_count,int(error is not None))
    def test_release_archive_contains_new_runtime_dependency(self):
        import package_alpha
        root=Path(__file__).resolve().parents[1]
        with tempfile.TemporaryDirectory() as d:
            fixture=Path(d)/'source';fixture.mkdir()
            names=subprocess.check_output(['git','ls-files'],cwd=root,text=True).splitlines()
            for name in names+['tools/support_report.py','tools/test_support_report.py']:
                source=root/name
                if source.is_file():
                    target=fixture/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(source,target)
            build=fixture/'.pio/build/ttgo_t_display';build.mkdir(parents=True)
            for name in ['bootloader.bin','partitions.bin','firmware.bin']:(build/name).write_bytes(b'SYNTHETIC-NOT-FIRMWARE')
            home=Path(d)/'home';boot=home/'.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin'
            boot.parent.mkdir(parents=True);boot.write_bytes(b'SYNTHETIC-NOT-FIRMWARE')
            with patch.object(package_alpha,'ROOT',fixture),patch.object(Path,'home',return_value=home),contextlib.redirect_stdout(io.StringIO()):package_alpha.main()
            with zipfile.ZipFile(next((fixture/'dist').glob('*.zip'))) as archive:
                self.assertEqual(archive.read('tools/support_report.py'),(root/'tools/support_report.py').read_bytes())
                self.assertIn('tools/tera_usb.py',archive.namelist())

if __name__=='__main__':unittest.main(verbosity=2)
