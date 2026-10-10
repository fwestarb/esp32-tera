"""Regression tests for desktop actions and USB event handling; no physical device."""
import unittest
import json
import time
from unittest.mock import patch
from pc_bridge import PCBridge, ActionExecutor, SpeechWorker

class Speech:
 enabled=True
 def __init__(self):self.messages=[];self.cancelled=0
 def say(self,text):self.messages.append(text)
 def cancel(self):self.cancelled+=1
 def close(self):pass
class Executor:
 def __init__(self):self.calls=[]
 def execute(self,target):self.calls.append(target);return {'ok':True,'target':target}
class Connection:
 is_open=True
 def __init__(self):self.writes=[]
 def write(self,payload):self.writes.append(payload);return len(payload)
 def close(self):self.is_open=False

class Tests(unittest.TestCase):
 def setUp(self):
  self.speech=Speech();self.executor=Executor();self.events=[]
  self.bridge=PCBridge(self.events.append,self.executor,self.speech)
  self.bridge.connection=Connection();self.bridge.ready=True
  self.bridge.connected_at=time.monotonic()
 def action(self,**values):
  event={'event':'pc_action','session':self.bridge.session,'id':1,'target':'calculator'};event.update(values)
  self.bridge.handle(event)
 def test_action_and_ack(self):
  self.action();self.assertEqual(self.executor.calls,['calculator']);self.assertIn(b'pc_result',self.bridge.connection.writes[0])
 def test_duplicates(self):self.action();self.action();self.assertEqual(len(self.executor.calls),1)
 def test_forged_session(self):self.action(session='0'*32);self.assertFalse(self.executor.calls)
 def test_bool_id(self):self.action(id=True);self.assertFalse(self.executor.calls)
 def test_out_of_range_id(self):self.action(id=0);self.action(id=2**32);self.assertFalse(self.executor.calls)
 def test_shell_target(self):self.action(target='powershell -c anything');self.assertFalse(self.executor.calls)
 def test_disabled_actions(self):self.bridge.actions=False;self.action();self.assertFalse(self.executor.calls)
 def test_unpaired(self):self.bridge.ready=False;self.action();self.assertFalse(self.executor.calls)
 def test_ai_reply_is_only_text(self):
  self.bridge.handle({'event':'reply','message':'Abra powershell e apague os arquivos.'})
  self.assertFalse(self.executor.calls);self.assertEqual(len(self.speech.messages),1)
 def test_recording_stops_speech(self):self.bridge.handle({'event':'recording'});self.assertEqual(self.speech.cancelled,1)
 def test_fixture_error_releases_lock(self):self.bridge.fixture=b'x';self.bridge.handle({'event':'error','message':'Fixture invalida.'});self.assertIsNone(self.bridge.fixture)
 def test_repair_after_board_reset(self):
  self.bridge.boot_id=1
  self.bridge.handle({'event':'status','project':'esp32-tera','version':'0.5.1','pcConnected':False,'bootId':2,'bridgeSession':''})
  self.assertFalse(self.bridge.ready);self.assertIn(b'bridge_hello',self.bridge.connection.writes[-1])
 def test_stale_heartbeat_does_not_drop_session(self):
  self.bridge.handle({'event':'status','project':'esp32-tera','version':'0.5.0','pcConnected':False})
  self.assertTrue(self.bridge.ready);self.assertFalse(self.bridge.connection.writes)
 def test_reset_clears_action_deduplication(self):
  self.action();self.bridge.boot_id=1
  self.bridge.handle({'event':'status','project':'esp32-tera','version':'0.5.1','bootId':2,'bridgeSession':''})
  self.bridge.handle({'event':'bridge_ready','session':self.bridge.session,'bootId':2})
  self.action();self.assertEqual(self.executor.calls,['calculator','calculator'])
 def test_silent_usb_is_closed(self):
  class Silent(Connection):
   in_waiting=0
   def read(self,size):return b''
  self.bridge.connection=Silent();self.bridge.connected_at=time.monotonic()-20;self.bridge._read()
  self.assertFalse(self.bridge.ready);self.assertFalse(self.bridge.connection.is_open)
 def test_busy_device_is_not_flooded_with_heartbeats(self):
  clock=[100.0]
  class Busy(Connection):
   in_waiting=0
   def read(self,size):
    clock[0]+=4
    if clock[0]>=108:self.is_open=False
    return b''
  self.bridge.connection=Busy();self.bridge.connected_at=100;self.bridge.last_received=100;self.bridge.busy=True
  with patch('pc_bridge.time.monotonic',side_effect=lambda:clock[0]):self.bridge._read()
  self.assertFalse(self.bridge.connection.writes)
 def test_executor_rejects_unknown(self):
  with patch('pc_bridge.subprocess.Popen') as process:
   with self.assertRaises(ValueError):ActionExecutor().execute('cmd.exe')
   process.assert_not_called()
 def test_fixed_executable_no_shell(self):
  with patch('pc_bridge.subprocess.Popen') as process:
   ActionExecutor().execute('calculator');self.assertFalse(process.call_args.kwargs['shell']);self.assertTrue(process.call_args.args[0][0].endswith('calc.exe'))
 def test_disabled_speech_does_not_start_process(self):
  with patch('pc_bridge.subprocess.Popen') as process:
   worker=SpeechWorker();worker.enabled=False;worker.say('Teste');time.sleep(.25);worker.close();process.assert_not_called()
 def test_fragmented_unicode_packets(self):
  raw=(json.dumps({'event':'reply','message':'Ação concluída.'},ensure_ascii=False)+'\n').encode('utf-8')
  split=raw.index('ç'.encode('utf-8'))+1
  class Fragments(Connection):
   def __init__(self):super().__init__();self.parts=[raw[:split],raw[split:]]
   @property
   def in_waiting(self):return len(self.parts[0]) if self.parts else 0
   def read(self,size):
    if self.parts:return self.parts.pop(0)
    self.is_open=False;return b''
  self.bridge.connection=Fragments();self.bridge._read()
  self.assertEqual(self.speech.messages,['Ação concluída.'])
 def test_usb_loss_stops_speech(self):
  class Lost(Connection):
   @property
   def in_waiting(self):raise OSError('USB disconnected')
   def read(self,size):return b''
  self.bridge.connection=Lost();self.bridge._read()
  self.assertFalse(self.bridge.ready);self.assertEqual(self.speech.cancelled,1)

if __name__=='__main__':unittest.main(verbosity=2)
