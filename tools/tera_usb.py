"""Windows USB companion: automatic connection, speech and fixed desktop actions."""
import argparse
import json
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import queue
import socket
import os
from pathlib import Path
import threading
import tkinter as tk
from tkinter import ttk
from urllib.request import Request, urlopen
from pc_bridge import PCBridge

IPC_PORT = 39135
VOICES={'Dora · feminina local (Kokoro)':'kokoro-dora','Faber · neural local (Piper)':'piper-faber','Daniel · Windows':'windows-daniel','Maria · Windows':'windows-maria'}
PREFERENCES=Path(os.environ.get('LOCALAPPDATA',str(Path.home())))/'ESP32 Tera'/'preferences.json'

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--minimized', action='store_true')
    parser.add_argument('--self-test', action='store_true', help=argparse.SUPPRESS)
    args = parser.parse_args()
    events = queue.Queue()
    bridge = PCBridge(events.put)
    try:
        selected_voice=json.loads(PREFERENCES.read_text(encoding='utf-8')).get('voice')
        if selected_voice in VOICES.values():bridge.speech.voice=selected_voice
    except (OSError,ValueError):pass
    class Handler(BaseHTTPRequestHandler):
        def do_GET(self):
            if self.path!='/health':self.send_error(404);return
            body=json.dumps({'app':'esp32-tera','usb':bridge.responsive(),'port':bridge.current_port,'speech':bridge.speech.enabled,'actions':bridge.actions,'voice':bridge.speech.voice,'firmware':bridge.last_status.get('version'),'error':bridge.last_error}).encode()
            self.send_response(200);self.send_header('Content-Type','application/json');self.send_header('Cache-Control','no-store');self.send_header('Content-Length',str(len(body)));self.end_headers();self.wfile.write(body)
        def do_POST(self):
            if self.path != '/show' or self.headers.get('Origin') or self.headers.get('X-Tera-App') != 'show':
                self.send_error(403); return
            events.put({'event':'show'}); self.send_response(204); self.end_headers()
        def log_message(self, *args): pass
    class IPCServer(ThreadingHTTPServer):
        allow_reuse_address = False
        def server_bind(self):
            if hasattr(socket,'SO_EXCLUSIVEADDRUSE'):
                self.socket.setsockopt(socket.SOL_SOCKET,socket.SO_EXCLUSIVEADDRUSE,1)
            super().server_bind()
    try: ipc = IPCServer(('127.0.0.1',IPC_PORT),Handler)
    except OSError:
        try: urlopen(Request(f'http://127.0.0.1:{IPC_PORT}/show',data=b'',headers={'X-Tera-App':'show'}),timeout=2).close()
        except OSError: pass
        bridge.close()
        return
    threading.Thread(target=ipc.serve_forever,daemon=True).start()
    root = tk.Tk(); root.title('ESP32 Tera — assistente USB'); root.geometry('820x850')
    frame = ttk.Frame(root,padding=16); frame.pack(fill='both',expand=True)
    ttk.Label(frame,text='ESP32 TERA',font=('Segoe UI',18,'bold')).pack(anchor='w')
    status = tk.StringVar(value='Procurando a TTGO por USB…')
    ttk.Label(frame,textvariable=status).pack(anchor='w',pady=4)
    row = ttk.Frame(frame); row.pack(fill='x',pady=6)
    port = ttk.Combobox(row,values=bridge.ports(),width=10); port.pack(side='left')
    if bridge.ports(): port.set(bridge.ports()[0])
    speech = tk.BooleanVar(value=True); actions = tk.BooleanVar(value=True); automatic = tk.BooleanVar(value=True)
    portal_active = False
    config_initialized = False
    def append(text):
        output.configure(state='normal'); output.insert('end',text+'\n\n')
        if int(output.index('end-1c').split('.')[0])>400: output.delete('1.0','101.0')
        output.see('end'); output.configure(state='disabled')
    def connect():
        try: bridge.connect(port.get())
        except OSError: status.set('Porta ocupada ou desconectada. Tentarei conectar novamente.')
    def send(payload):
        if bridge.busy: append('Aguarde a operação atual.'); return False
        if not bridge.send(payload): append('Conecte a TTGO por USB.'); return False
        return True
    def preferences():
        bridge.actions=actions.get(); bridge.speech.enabled=speech.get()
        if not speech.get(): bridge.speech.cancel()
        if bridge.ready: bridge.send({'op':'bridge_ping','session':bridge.session,'actions':bridge.actions,'speech':bridge.speech.enabled})
    ttk.Button(row,text='Conectar',command=connect).pack(side='left',padx=6)
    ttk.Button(row,text='Estado',command=lambda:send({'op':'status'})).pack(side='left')
    ttk.Button(row,text='Configurar pelo celular',command=lambda:send({'op':'portal','enabled':not portal_active})).pack(side='left',padx=6)
    options = ttk.Frame(frame); options.pack(fill='x',pady=6)
    ttk.Checkbutton(options,text='Falar respostas no PC',variable=speech,command=preferences).pack(side='left')
    ttk.Checkbutton(options,text='Permitir comandos de abertura',variable=actions,command=preferences).pack(side='left',padx=8)
    ttk.Checkbutton(options,text='Conexão automática',variable=automatic).pack(side='left')
    voice_row=ttk.Frame(frame);voice_row.pack(fill='x',pady=3)
    ttk.Label(voice_row,text='Voz:').pack(side='left')
    voice=ttk.Combobox(voice_row,values=list(VOICES),state='readonly',width=31)
    voice.set(next(label for label,key in VOICES.items() if key==bridge.speech.voice));voice.pack(side='left',padx=6)
    def change_voice(event=None):
        bridge.speech.cancel();bridge.speech.voice=VOICES[voice.get()]
        try:
            PREFERENCES.parent.mkdir(parents=True,exist_ok=True)
            PREFERENCES.write_text(json.dumps({'voice':bridge.speech.voice}),encoding='utf-8')
        except OSError:append('Não foi possível salvar a escolha de voz.')
    voice.bind('<<ComboboxSelected>>',change_voice)
    ttk.Button(voice_row,text='Ouvir exemplo',command=lambda:bridge.speech.say('Olá, eu sou a Tera. Esta é minha voz em português.')).pack(side='left')
    ttk.Label(frame,text='Comandos: “pode abrir a calculadora?”, “abra o Bloco de notas”, “abrir GitHub”,\n“abrir YouTube”, “abrir navegador” ou “abrir explorador de arquivos”.').pack(anchor='w',pady=5)
    config = ttk.LabelFrame(frame,text='Configuração da placa',padding=10); config.pack(fill='x',pady=5)
    provider = ttk.Combobox(config,values=['groq','openrouter'],state='readonly'); provider.set('groq')
    ttk.Label(config,text='IA: Groq para texto e voz; OpenRouter para texto').pack(anchor='w'); provider.pack(fill='x')
    model=ttk.Combobox(config,values=['openai/gpt-oss-120b','openai/gpt-oss-20b'],state='readonly');model.set('openai/gpt-oss-120b')
    ttk.Label(config,text='Modelo de conversa (não altera a transcrição)').pack(anchor='w');model.pack(fill='x')
    def provider_models(event=None):
        groq=provider.get()=='groq';model.configure(values=['openai/gpt-oss-120b','openai/gpt-oss-20b'] if groq else ['openrouter/free'],state='readonly' if groq else 'normal');model.set('openai/gpt-oss-120b' if groq else 'openrouter/free')
    provider.bind('<<ComboboxSelected>>',provider_models)
    ttk.Button(config,text='Trocar somente o modelo',command=lambda:send({'op':'set_model','model':model.get()})).pack(anchor='w')
    fields={}
    for name,label,secret in [('ssid','Wi-Fi 2,4 GHz',False),('password','Senha do Wi-Fi',True),('key','Chave de API',True)]:
        ttk.Label(config,text=label).pack(anchor='w')
        fields[name]=ttk.Entry(config,show='•' if secret else ''); fields[name].pack(fill='x')
    ttk.Label(config,text='Senha/chave vazias mantêm os dados da mesma rede/provedor.\nAs chaves ficam na NVS da placa, sem criptografia; não são gravadas no PC.').pack(anchor='w',pady=4)
    def configure():
        payload={name:field.get() for name,field in fields.items()}; payload.update(op='configure',provider=provider.get(),model=model.get(),keepBlank=True)
        if send(payload): fields['password'].delete(0,'end'); fields['key'].delete(0,'end')
    ttk.Button(config,text='Salvar na placa',command=configure).pack(anchor='w')
    ttk.Label(frame,text='Pergunta ou comando · até 1024 bytes · contexto da última troca').pack(anchor='w',pady=(8,0))
    prompt=ttk.Entry(frame); prompt.pack(fill='x',pady=5)
    def chat(event=None):
        text=prompt.get().strip()
        if not text or len(text.encode('utf-8'))>1024: append('Digite uma mensagem de 1 a 1024 bytes.'); return
        if send({'op':'chat','text':text}): append('Você: '+text)
    prompt.bind('<Return>',chat)
    buttons=ttk.Frame(frame); buttons.pack(fill='x')
    ttk.Button(buttons,text='Enviar',command=chat).pack(side='left')
    ttk.Button(buttons,text='Limpar conversa',command=lambda:send({'op':'clear_history'})).pack(side='left',padx=8)
    ttk.Button(buttons,text='Parar voz',command=bridge.speech.cancel).pack(side='left')
    output=tk.Text(frame,height=9,wrap='word',state='disabled'); output.pack(fill='both',expand=True,pady=8)
    def poll():
        nonlocal portal_active, config_initialized
        while not events.empty():
            event=events.get(); kind=event.get('event')
            if kind=='show': root.deiconify(); root.lift()
            elif kind=='status':
                portal_active=bool(event.get('portal'))
                if not config_initialized and event.get('provider'):
                    provider.set(event['provider']);provider_models();model.set(event.get('model',model.get()));fields['ssid'].insert(0,event.get('ssid',''));config_initialized=True
                status.set(f"{bridge.current_port} · firmware {event.get('version')} · {event.get('model','')} · Wi-Fi {'conectado' if event.get('wifi') else 'pendente'}"+(' · configuração pelo celular aberta' if portal_active else ''))
            elif kind=='usb': status.set('USB conectado; identificando Tera…' if event.get('connected') else 'USB desconectado. Aguardando a placa…')
            elif kind=='transcript': append('Você disse: '+event.get('message',''))
            elif kind in ('reply','error','configured','network','countdown'):
                append(('Tera: ' if kind=='reply' else '')+event.get('message',''))
                if kind=='network': bridge.send({'op':'status'})
            elif kind=='recording': append('Gravando… solte GPIO35 para enviar.')
            elif kind=='busy': append(event.get('message','Consultando IA…'))
            elif kind=='speech' and not event.get('ok'): append('Não foi possível falar: '+event.get('error','verifique a saída de áudio do Windows.'))
            elif kind=='action_result': append('Abertura solicitada no PC.' if event.get('ok') else 'Falha ao abrir: '+event.get('error',''))
        root.after(100,poll)
    def auto_connect():
        ports=bridge.ports(); port.configure(values=ports)
        if automatic.get() and (not bridge.connection or not bridge.connection.is_open) and ports: port.set(ports[0]); connect()
        root.after(3000,auto_connect)
    def close(): bridge.close(); ipc.shutdown(); ipc.server_close(); root.destroy()
    ttk.Button(buttons,text='Encerrar assistente',command=close).pack(side='right')
    root.protocol('WM_DELETE_WINDOW',root.iconify)
    if args.self_test:
        def ui_test():
            try:
                root.update_idletasks()
                report={'passed':True,'window':{'width':root.winfo_width(),'height':root.winfo_height()},'checks':[]}
                assert output.winfo_height()>=40,'Área de respostas sem espaço'
                assert buttons.winfo_rooty()+buttons.winfo_height()<=root.winfo_rooty()+root.winfo_height(),'Botões fora da janela'
                for name,control in fields.items():
                    assert control.winfo_width()>100
                    if name in ('password','key'):assert control.cget('show')=='•'
                report['checks'].extend(['controls fit window','secret fields masked','response area visible'])
                provider.set('openrouter');provider_models();assert model.get()=='openrouter/free'
                provider.set('groq');provider_models();assert model.get()=='openai/gpt-oss-120b'
                model.set('openai/gpt-oss-20b')
                events.put({'event':'status','provider':'groq','model':'openai/gpt-oss-120b','version':'1.0.0-alpha.1','wifi':True})
                poll();assert model.get()=='openai/gpt-oss-20b','Estado sobrescreveu escolha do usuário'
                assert VOICES[voice.get()]==bridge.speech.voice
                report['checks'].extend(['provider changes compatible models','status preserves edited selection','voice selection matches engine'])
                (Path(__file__).resolve().parents[1]/'docs/TESTE-INTERFACE-PC-ALPHA.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
                print('PASS: Tk companion UI (6 checks)',flush=True)
            except Exception as error:
                print('FAIL: '+str(error),flush=True);root.exit_code=1
            finally:close()
        root.after(4500,ui_test)
    if args.minimized: root.after(100,root.iconify)
    poll(); auto_connect(); root.mainloop()
    if getattr(root,'exit_code',0):raise SystemExit(root.exit_code)

if __name__=='__main__': main()
