"""USB companion: offline Windows speech and explicit, fixed desktop actions.

AI replies are spoken as text and NEVER interpreted as executable commands.
"""
from collections import deque
import json
import os
from pathlib import Path
import queue
import subprocess
import threading
import time
import uuid
import webbrowser
import serial
from serial.tools import list_ports

ROOT=Path(__file__).resolve().parent
PIPER_PYTHON=ROOT.parent/'.venv-tts/Scripts/python.exe'
DORA_PYTHON=ROOT.parent/'.venv-dora/Scripts/python.exe'
DORA_MODEL=ROOT.parent/'models/kokoro/kokoro-v1.0.onnx'
PIPER_MODEL=ROOT.parent/'models/piper/pt_BR-faber-medium.onnx'
TARGETS={'calculator':'a calculadora','notepad':'o Bloco de notas','explorer':'o explorador de arquivos','browser':'o navegador','github':'o GitHub','youtube':'o YouTube'}
URLS={'browser':'about:blank','github':'https://github.com/fwestarb','youtube':'https://www.youtube.com/'}

class ActionExecutor:
    def __init__(self):self.last_process=None
    def execute(self,target):
        if target not in TARGETS:raise ValueError('Acao nao permitida.')
        if target in URLS:
            if not webbrowser.open(URLS[target],new=2):raise OSError('Navegador indisponivel.')
            return {'ok':True,'target':target,'method':'default-browser'}
        binary={'calculator':'calc.exe','notepad':'notepad.exe','explorer':'explorer.exe'}[target]
        executable=Path(os.environ.get('WINDIR',r'C:\Windows'))/'System32'/binary
        self.last_process=subprocess.Popen([str(executable)],shell=False)
        return {'ok':True,'target':target,'pid':self.last_process.pid}

class SpeechWorker:
    def __init__(self,callback=lambda event:None):
        self.callback=callback;self.enabled=True;self.queue=queue.Queue(maxsize=3)
        self.lock=threading.Lock();self.process=None;self.closed=False;self.generation=0
        self.voice='kokoro-dora' if DORA_PYTHON.exists() and DORA_MODEL.exists() else 'piper-faber' if PIPER_PYTHON.exists() and PIPER_MODEL.exists() else 'windows-daniel'
        self.thread=threading.Thread(target=self._run,daemon=True);self.thread.start()
    def say(self,text):
        if self.closed or not self.enabled or not isinstance(text,str) or not text.strip():return
        text=text[:4000]
        if self.queue.full():
            try:self.queue.get_nowait()
            except queue.Empty:pass
        self.queue.put_nowait((self.generation,text))
    def cancel(self):
        while not self.queue.empty():
            try:self.queue.get_nowait()
            except queue.Empty:break
        with self.lock:
            self.generation+=1
            if self.process and self.process.poll() is None:self.process.terminate()
    def _run(self):
        while not self.closed:
            try:generation,text=self.queue.get(timeout=.2)
            except queue.Empty:continue
            if not self.enabled:continue
            try:
                with self.lock:
                    if self.closed or generation!=self.generation:continue
                    selected=self.voice
                    request={'text':text}
                    if selected=='kokoro-dora':
                        command=[str(DORA_PYTHON),str(ROOT/'dora_speech.py')]
                    elif selected=='piper-faber':
                        command=[str(PIPER_PYTHON),str(ROOT/'piper_speech.py')]
                    else:
                        command=['powershell.exe','-NoProfile','-NonInteractive','-File',str(ROOT/'speech.ps1')]
                        request['voice']='Microsoft Daniel' if selected=='windows-daniel' else 'Microsoft Maria Desktop'
                    self.process=subprocess.Popen(command,stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,encoding='utf-8',creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0),env={**os.environ,'PYTHONIOENCODING':'utf-8'})
                    process=self.process
                out,err=process.communicate(json.dumps(request,ensure_ascii=False),timeout=180)
                if process.returncode==0:
                    result=json.loads(out.strip());self.callback({'event':'speech',**result})
                elif process.returncode not in (-15,1):self.callback({'event':'speech','ok':False,'error':'Fala interrompida.'})
                elif out.strip():self.callback({'event':'speech',**json.loads(out.strip())})
            except Exception as error:
                self.callback({'event':'speech','ok':False,'error':str(error)[:200]})
            finally:
                with self.lock:
                    if self.process and self.process.poll() is None:self.process.kill()
                    self.process=None
    def close(self):self.closed=True;self.cancel();self.thread.join(timeout=2)

class PCBridge:
    def __init__(self,callback=lambda event:None,executor=None,speech=None):
        self.callback=callback;self.executor=executor or ActionExecutor();self.speech=speech or SpeechWorker(callback)
        self.connection=None;self.closed=False;self.ready=False;self.busy=False;self.actions=True
        self.session=uuid.uuid4().hex;self.seen=deque(maxlen=128);self.writer=threading.Lock()
        self.fixture=None;self.buffer=bytearray();self.thread=None;self.last_hello=0
        self.current_port=None;self.auto=False
        self.last_received=0;self.connected_at=0;self.last_error='';self.last_status={};self.boot_id=None
    def ports(self):return [p.device for p in list_ports.comports() if p.vid==0x10c4 and p.pid==0xea60]
    def connect(self,port):
        self.disconnect();self.session=uuid.uuid4().hex;self.seen.clear();self.buffer.clear()
        conn=serial.Serial(port=None,baudrate=115200,timeout=.15,write_timeout=20)
        conn.dtr=False;conn.rts=False;conn.port=port;conn.open()
        self.connection=conn;self.current_port=port;self.ready=False;self.busy=False;self.closed=False
        self.connected_at=time.monotonic();self.last_received=0;self.last_error='';self.last_status={};self.boot_id=None;self.last_hello=0
        self.thread=threading.Thread(target=self._read,daemon=True);self.thread.start()
        self.send({'op':'status'},force=True)
        self.callback({'event':'usb','connected':True,'port':port})
    def send(self,payload,force=False):
        if not self.connection or not self.connection.is_open:return False
        if self.fixture is not None and not force:return False
        with self.writer:
            try:self.connection.write((json.dumps(payload,ensure_ascii=False)+'\n').encode('utf-8'));return True
            except (serial.SerialException,OSError):return False
    def _hello(self):
        self.last_hello=time.monotonic()
        self.send({'op':'bridge_hello','session':self.session,'actions':self.actions,'speech':self.speech.enabled})
    def responsive(self):
        limit=180 if self.busy else 15
        return bool(self.ready and self.connection and self.connection.is_open and self.last_received and time.monotonic()-self.last_received<limit)
    def handle(self,event):
        if not isinstance(event,dict):return
        self.last_received=time.monotonic()
        kind=event.get('event')
        if kind=='status':
            self.last_status=event.copy()
            if event.get('project')!='esp32-tera' or event.get('version') not in ('0.5.0','0.5.1','1.0.0-alpha.1'):
                self.callback({'event':'error','message':'Instale o firmware Tera Alpha 1.0 para usar o assistente do PC.'});return
            boot=event.get('bootId')
            if boot is not None:
                if self.boot_id is not None and boot!=self.boot_id:
                    self.session=uuid.uuid4().hex;self.seen.clear();self.ready=False
                self.boot_id=boot
                if event.get('bridgeSession')!=self.session:self.ready=False
            # A stale device heartbeat during a blocking cloud request does not
            # mean its session changed. Re-pair only on an explicit session error.
            if not self.ready and time.monotonic()-self.last_hello>.8:self._hello()
        elif kind=='bridge_ready':
            if event.get('session',self.session)!=self.session:return
            self.ready=True;self.boot_id=event.get('bootId',self.boot_id)
        elif kind in ('busy','recording','countdown'):
            self.busy=True
            if kind in ('recording','countdown'):self.speech.cancel()
        elif kind in ('reply','error','configured','network'):
            self.busy=False
            if kind=='error':
                self.fixture=None
                if event.get('message')=='Sessao do PC invalida.':
                    self.ready=False;self.session=uuid.uuid4().hex;self.seen.clear();self._hello()
            if kind=='reply' and self.ready:self.speech.say(event.get('message',''))
        elif kind=='pc_action':
            target=event.get('target');action_id=event.get('id')
            if not self.ready or not self.actions or event.get('session')!=self.session or type(action_id) is not int or not 1<=action_id<=0xffffffff or target not in TARGETS:return
            if action_id in self.seen:return
            self.seen.append(action_id)
            try:result=self.executor.execute(target)
            except Exception as error:result={'ok':False,'target':target,'error':str(error)[:200]}
            self.callback({'event':'action_result',**result})
            self.send({'op':'pc_result','session':self.session,'id':action_id,'ok':result['ok']})
        elif kind=='fixture_ready' and self.fixture is not None:
            with self.writer:
                payload=self.fixture
                try:
                    if self.connection.write(payload)!=len(payload):raise OSError('Gravacao de teste incompleta.')
                finally:self.fixture=None
        self.callback(event)
    def send_fixture(self,payload):
        import zlib
        if not self.ready or self.fixture is not None or not 4000<=len(payload)<=80000:return False
        self.fixture=payload
        if not self.send({'op':'voice_fixture','samples':len(payload),'crc':zlib.crc32(payload)},force=True):self.fixture=None;return False
        return True
    def _read(self):
        conn=self.connection;heartbeat=time.monotonic()+3
        try:
            while not self.closed and conn.is_open and self.connection is conn:
                chunk=conn.read(min(4096,conn.in_waiting or 1))
                self.buffer.extend(chunk)
                while b'\n' in self.buffer:
                    line,_,rest=self.buffer.partition(b'\n');self.buffer=bytearray(rest)
                    try:self.handle(json.loads(line.decode('utf-8')))
                    except (ValueError,UnicodeDecodeError):pass
                if len(self.buffer)>32768:self.buffer.clear()
                now=time.monotonic()
                if self.fixture is None and now>=heartbeat:
                    if not self.busy:
                        if self.ready:self.send({'op':'bridge_ping','session':self.session,'speech':self.speech.enabled,'actions':self.actions})
                        self.send({'op':'status'})
                    heartbeat=now+3
                if self.fixture is None and now-(self.last_received or self.connected_at)>(180 if self.busy else 15):
                    raise OSError('A placa parou de responder ao USB.')
        except (serial.SerialException,OSError):
            self.ready=False;self.busy=False;self.fixture=None
            self.last_error='A placa nao responde ou o USB foi desconectado. Tentando reconectar.'
            self.speech.cancel()
            self.callback({'event':'usb','connected':False,'port':self.current_port})
            self.callback({'event':'error','message':self.last_error})
        finally:
            self.ready=False
            if conn.is_open:conn.close()
    def disconnect(self):
        self.closed=True;self.ready=False;self.fixture=None
        # Stop overlapped Windows I/O before closing its handles.
        if self.connection and self.connection.is_open:
            try:
                if hasattr(self.connection,'cancel_read'):self.connection.cancel_read()
                if hasattr(self.connection,'cancel_write'):self.connection.cancel_write()
            except OSError:pass
        if self.thread and self.thread is not threading.current_thread():self.thread.join(timeout=2)
        if self.connection and self.connection.is_open:
            try:self.connection.close()
            except OSError:pass
        self.connection=None;self.speech.cancel()
    def close(self):self.disconnect();self.speech.close()
