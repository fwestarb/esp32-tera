"""Local USB configuration/chat. Credentials are never written to a PC file."""
import json
import queue
import threading
import tkinter as tk
from tkinter import ttk, messagebox
import serial
from serial.tools import list_ports

root = tk.Tk()
root.title('ESP32 Tera — configuração e chat USB')
root.geometry('700x620')
events = queue.Queue()
connection = None
busy = False
frame = ttk.Frame(root, padding=16)
frame.pack(fill='both', expand=True)
ttk.Label(frame, text='ESP32 TERA', font=('Segoe UI', 18, 'bold')).pack(anchor='w')
ttk.Label(frame, text='Wi-Fi 2,4 GHz • voz com Groq • segure GPIO35 para falar e solte para enviar').pack(anchor='w')
row = ttk.Frame(frame); row.pack(fill='x', pady=10)
port = ttk.Combobox(row, values=[p.device for p in list_ports.comports()], width=12)
port.pack(side='left')
ports = list_ports.comports()
if ports: port.set(next((p.device for p in ports if p.vid == 0x10c4), ports[0].device))

def send(payload):
    if not connection or not connection.is_open:
        messagebox.showerror('USB', 'Conecte à placa primeiro.'); return False
    if busy:
        messagebox.showinfo('IA', 'Aguarde a consulta atual.'); return False
    try:
        connection.write((json.dumps(payload, ensure_ascii=False)+'\n').encode('utf-8'))
        return True
    except serial.SerialException:
        messagebox.showerror('USB', 'Conexão perdida. Reconecte a placa.'); return False

def read_loop(conn):
    while conn.is_open:
        try:
            line = conn.readline()
            if line:
                try: events.put(json.loads(line.decode('utf-8')))
                except (ValueError, UnicodeDecodeError): pass
        except serial.SerialException:
            events.put({'event': 'error', 'message': 'USB desconectado.'}); break

def connect():
    global connection, busy
    if connection: connection.close()
    try:
        connection = serial.Serial(port=None,baudrate=115200,timeout=1)
        connection.dtr=False; connection.rts=False;connection.port=port.get();connection.open()
        busy=False
        threading.Thread(target=read_loop,args=(connection,),daemon=True).start()
        root.after(2000,lambda: send({'op':'status'}))
    except serial.SerialException:
        messagebox.showerror('USB', 'Não foi possível abrir a porta. Feche o monitor serial e tente novamente.')

ttk.Button(row,text='Conectar',command=connect).pack(side='left',padx=10)
ttk.Button(row,text='Verificar estado',command=lambda:send({'op':'status'})).pack(side='left')
ttk.Button(row,text='Diagnosticar Wi-Fi',command=lambda:send({'op':'scan'})).pack(side='left',padx=6)
ttk.Button(row,text='Calibrar ruído',command=lambda:send({'op':'calibrate_noise'})).pack(side='left')
fields={}
ttk.Label(frame,text='Provedor de IA (use uma conta Groq no plano Free)').pack(anchor='w')
provider=ttk.Combobox(frame,values=['groq','openrouter'],state='readonly')
provider.set('groq');provider.pack(fill='x',pady=(0,7))
for name,label,masked in [('ssid','Nome da rede Wi-Fi',False),('password','Senha do Wi-Fi',True),('key','Chave do provedor: Groq gsk_... / OpenRouter sk-or-...',True)]:
    ttk.Label(frame,text=label).pack(anchor='w')
    field=ttk.Entry(frame,show='•' if masked else '')
    field.pack(fill='x',pady=(0,7)); fields[name]=field

def configure():
    payload={name:entry.get() for name,entry in fields.items()}
    payload['op']='configure'
    payload['provider']=provider.get()
    if send(payload):
        fields['password'].delete(0,'end'); fields['key'].delete(0,'end')

ttk.Button(frame,text='Salvar configuração na placa',command=configure).pack(anchor='w')
ttk.Label(frame,text='A chave fica na memória da placa (NVS sem criptografia). Não compartilhe backups da flash.').pack(anchor='w',pady=7)
ttk.Separator(frame).pack(fill='x',pady=6)
ttk.Label(frame,text='Mensagem (até 1024 bytes; cada envio é uma pergunta independente)').pack(anchor='w')
prompt=ttk.Entry(frame); prompt.pack(fill='x',pady=6)
def chat():
    text=prompt.get().strip()
    if not text or len(text.encode('utf-8'))>1024:
        messagebox.showerror('Mensagem','Digite de 1 a 1024 bytes de texto.');return
    send({'op':'chat','text':text})
chat_button=ttk.Button(frame,text='Enviar à IA',command=chat,state='disabled')
chat_button.pack(anchor='w')
output=tk.Text(frame,height=12,wrap='word',state='disabled');output.pack(fill='both',expand=True,pady=8)
def append(text):
    output.configure(state='normal');output.insert('end',text+'\n\n');output.see('end');output.configure(state='disabled')
def poll():
    global busy
    while not events.empty():
        event=events.get();kind=event.get('event')
        if kind=='status':
            chat_button.configure(state='normal' if event.get('wifi') and event.get('configured') else 'disabled')
            append(event.get('networkMessage',f"Wi-Fi {'conectado' if event.get('wifi') else 'pendente'}")+f" Firmware {event.get('version')}.")
        elif kind in ('busy','recording','countdown'):
            busy=True;append(event.get('message','Gravando…' if kind=='recording' else 'Consultando IA…'))
        elif kind=='transcript': append('Você disse: '+event.get('message',''))
        elif kind in ('reply','error','configured','network'):
            busy=False;append(('Tera: ' if kind=='reply' else '')+event.get('message',''))
            if kind=='configured':
                chat_button.configure(state='disabled')
                root.after(5000,lambda:send({'op':'status'}))
    root.after(100,poll)
def close():
    if connection: connection.close()
    root.destroy()
root.protocol('WM_DELETE_WINDOW',close)
poll();root.mainloop()
