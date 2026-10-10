#include <Arduino.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <time.h>
#include <esp_task_wdt.h>
#include <esp_system.h>
#include "Board.h"
#include "Settings.h"
#include "VoiceAudio.h"
#include "Portal.h"
// Certificate verification plus the voice/command call chain exceeds 8 KB.
SET_LOOP_TASK_STACK_SIZE(16384);
extern const uint8_t caRoots[] asm("_binary_data_cert_openrouter_roots_pem_start");
TFT_eSPI screen;
Settings settings;
bool applyConfiguration(JsonObjectConst doc,bool keepBlank,String& error);
Portal portal(settings,[](JsonObjectConst doc,String& error){return applyConfiguration(doc,true,error);});
uint32_t reconnectAt=0;
String pcSession,pcPending,historyUser,historyAnswer;
uint32_t pcLastSeen=0,pcActionId=0,pcDeadline=0;
bool pcActions=true,pcSpeech=true,portalAttempted=false,menuLong=false;
uint32_t menuStarted=0;
uint32_t bootId=0;
bool watchdogEnabled=false;
void feedWatchdog(){if(watchdogEnabled)esp_task_wdt_reset();}
String input, answer, notice = "Configure pelo USB";
unsigned page=0;
bool dropping=false,lastButton=false;
uint32_t lastPress=0;
volatile uint8_t disconnectReason=0;
uint32_t wifiStarted=0;
bool wifiReported=false;
bool talkArmed=false;
double noiseRms=4.6;
String wifiDiagnosis(){
 if(WiFi.status()==WL_CONNECTED)return "WiFi conectado.";
 if(!settings.wifiSsid.length())return "WiFi nao configurado.";
 if(millis()-wifiStarted<20000)return "Conectando WiFi; aguarde ate 20 segundos.";
 switch(disconnectReason){
 case 201:return "Rede nao encontrada. Confira o nome exato, sinal e WiFi 2.4 GHz.";
 case 202:case 15:case 204:return "Falha de autenticacao/handshake. Confira senha e seguranca do roteador (WPA2).";
 default:return "WiFi desconectado; motivo ESP32 "+String(disconnectReason)+". Confira rede, sinal e roteador.";
 }
}
constexpr uint16_t bg=0x1082,green=0xA6EB,moss=0x6C88;
String displayText(String s) {
 const char* from[]={"á","à","ã","â","é","ê","í","ó","ô","õ","ú","ç","Á","É","Ã","Ç"};
 const char* to[]={"a","a","a","a","e","e","i","o","o","o","u","c","A","E","A","C"};
 for(unsigned i=0;i<16;i++) s.replace(from[i],to[i]);
 String clean;
 for(unsigned i=0;i<s.length();i++){unsigned char c=s[i];if(c>=32&&c<127)clean+=char(c);else if(c=='\n'||c=='\r'||c=='\t')clean+=' ';}
 return clean;
}
void render(){
 screen.fillScreen(bg);screen.setTextColor(green,bg);screen.drawString("ESP32 TERA",35,5,2);
 screen.drawLine(17,31,17,8,moss);screen.drawLine(17,22,8,13,moss);screen.drawLine(17,17,26,8,moss);
 if(answer.length()){screen.fillCircle(8,13,3,green);screen.fillCircle(26,8,3,green);}
 screen.setTextColor(TFT_LIGHTGREY,bg);
 String text=displayText(answer.length()?answer:notice);
 unsigned pages=max(1U,(text.length()+179)/180);page%=pages;
 for(unsigned row=0;row<6;row++)screen.drawString(text.substring(page*180+row*30,page*180+(row+1)*30),8,36+row*12,1);
 screen.setTextColor(moss,bg);screen.drawString("35: falar | 0: pagina "+String(page+1)+"/"+String(pages),8,119,1);
}
void emit(const char* type,const String& message=""){
 DynamicJsonDocument doc(256+message.length());doc["event"]=type;if(message.length())doc["message"]=message;serializeJson(doc,Serial);Serial.println();
}
void status(){
 StaticJsonDocument<1536> doc;doc["event"]="status";doc["version"]="0.5.1";doc["project"]="esp32-tera";
 doc["bootId"]=bootId;doc["bridgeSession"]=pcSession;doc["watchdogSeconds"]=watchdogEnabled?120:0;
 doc["networkMessage"]=wifiDiagnosis();doc["disconnectReason"]=disconnectReason;
 doc["configured"]=settings.wifiSsid.length()>0&&settings.apiKey.length()>0;doc["wifi"]=WiFi.status()==WL_CONNECTED;
 doc["freeHeap"]=ESP.getFreeHeap();doc["microphone"]=true;doc["voiceReady"]=!portal.active()&&settings.provider=="groq"&&settings.apiKey.length()>0&&WiFi.status()==WL_CONNECTED&&time(nullptr)>=1700000000;doc["noiseRms"]=noiseRms;doc["keyboard"]=false;doc["speaker"]=false;doc["provider"]=settings.provider;doc["model"]=settings.provider=="groq"?"openai/gpt-oss-20b":"google/gemma-4-26b-a4b-it:free";
 doc["stackWatermark"]=uxTaskGetStackHighWaterMark(nullptr);
 doc["clockReady"]=time(nullptr)>=1700000000;doc["portal"]=portal.active();doc["portalName"]=portal.active()?portal.name():"";doc["portalAddress"]=portal.active()?"192.168.4.1":"";
 doc["pcConnected"]=pcSession.length()>0&&millis()-pcLastSeen<10000;doc["pcSpeech"]=pcSpeech;doc["pcActions"]=pcActions;doc["history"]=historyUser.length()>0;
 serializeJson(doc,Serial);Serial.println();
}
void connectWifi(){
 wifiStarted=millis();wifiReported=false;disconnectReason=0;
 WiFi.mode(portal.active()?WIFI_AP_STA:WIFI_STA);if(settings.wifiSsid.length()){WiFi.begin(settings.wifiSsid.c_str(),settings.wifiPassword.c_str());configTime(0,0,"pool.ntp.org","time.google.com");notice="Conectando WiFi. Depois segure GPIO35 para falar.";}
}
bool applyConfiguration(JsonObjectConst doc,bool keepBlank,String& error){
 Settings next;if(!Settings::parse(doc,settings,keepBlank,next,error))return false;
 if(next.wifiSsid==settings.wifiSsid&&next.wifiPassword==settings.wifiPassword&&next.apiKey==settings.apiKey&&next.provider==settings.provider){reconnectAt=millis()+1500;return true;}
 if(!next.save()){error="Falha ao salvar na placa.";return false;}
 settings=next;historyUser="";historyAnswer="";reconnectAt=millis()+1500;return true;
}
void showPortal(){
 if(!portal.start()){notice="Falha ao abrir configuracao.";render();emit("error",notice);return;}
 answer="";page=0;notice="CONFIGURAR PELO CELULAR Rede: "+portal.name()+" Senha: "+portal.password()+" Abra http://192.168.4.1 . Segure GPIO0 para fechar.";render();emit("network","Configuracao pelo celular aberta. Rede: "+portal.name()+". Veja a senha na tela da TTGO; abra http://192.168.4.1.");
}
String classifyAction(String text){
 text=displayText(text);text.toLowerCase();text.trim();while(text.endsWith(".")||text.endsWith("!")||text.endsWith("?"))text.remove(text.length()-1);
 while(text.indexOf("  ")>=0)text.replace("  "," ");
 if(text.startsWith("tera "))text.remove(0,5);if(text.startsWith("por favor "))text.remove(0,10);
 if(text.startsWith("abra "))text.remove(0,5);else if(text.startsWith("abrir "))text.remove(0,6);else return "";
 if(text.startsWith("o ")||text.startsWith("a "))text.remove(0,2);
 if(text=="calculadora")return "calculator";
 if(text=="bloco de notas")return "notepad";
 if(text=="explorador"||text=="explorador de arquivos")return "explorer";
 if(text=="navegador")return "browser";
 if(text=="github")return "github";
 if(text=="youtube")return "youtube";
 return "";
}
String actionLabel(const String& target){
 if(target=="calculator")return "a calculadora";if(target=="notepad")return "o Bloco de notas";if(target=="explorer")return "o explorador de arquivos";if(target=="github")return "o GitHub";if(target=="youtube")return "o YouTube";return "o navegador";
}
void chat(const String& prompt);
String boundedUtf8(const String& text,unsigned limit){
 if(text.length()<=limit)return text;
 while(limit&&((uint8_t)text[limit]&0xc0)==0x80)limit--;
 return text.substring(0,limit);
}
void dispatchInput(const String& text){
 String action=classifyAction(text);if(!action.length()){chat(text);return;}
 if(pcPending.length()){emit("error","Aguarde a acao atual no PC.");return;}
 if(!pcSession.length()||!pcActions){answer="";notice="Abra o aplicativo Tera no PC e ative os comandos.";render();emit("error",notice);return;}
 pcPending=action;pcDeadline=millis()+5000;pcActionId++;answer="";notice="Solicitando abertura no PC...";render();emit("busy",notice);
 StaticJsonDocument<256> doc;doc["event"]="pc_action";doc["session"]=pcSession;doc["id"]=pcActionId;doc["target"]=action;serializeJson(doc,Serial);Serial.println();
}
void chat(const String& prompt){
 feedWatchdog();
 if(portal.active()){emit("error","Feche a configuracao pelo celular antes de consultar a IA.");return;}
 if(!settings.apiKey.length()){emit("error","Configure sua chave primeiro.");return;}
 if(WiFi.status()!=WL_CONNECTED){emit("error",wifiDiagnosis());return;}
 if(time(nullptr)<1700000000){emit("error","Aguarde a sincronizacao do relogio para HTTPS.");return;}
 answer="";page=0;notice="Consultando IA...";render();emit("busy");
 WiFiClientSecure client;client.setCACert(reinterpret_cast<const char*>(caRoots));client.setHandshakeTimeout(15);
 HTTPClient http;http.useHTTP10(true);http.setReuse(false);http.setConnectTimeout(15000);http.setTimeout(45000);
 if(!http.begin(client,settings.provider=="groq"?"https://api.groq.com/openai/v1/chat/completions":"https://openrouter.ai/api/v1/chat/completions")){emit("error","Falha ao iniciar HTTPS.");return;}
 http.addHeader("Authorization","Bearer "+settings.apiKey);http.addHeader("Content-Type","application/json");
 const bool groq=settings.provider=="groq";
 DynamicJsonDocument request(6144);request["model"]=groq?"openai/gpt-oss-20b":"google/gemma-4-26b-a4b-it:free";request["max_tokens"]=groq?1024:320;
 if(groq){request["reasoning_effort"]="low";request["include_reasoning"]=false;}
 JsonArray messages=request.createNestedArray("messages");
 JsonObject system=messages.createNestedObject();system["role"]="system";system["content"]="Voce e Tera, um assistente de chat. Responda em portugues de forma breve e clara. As mensagens podem ser transcricoes de voz ja realizadas; responda ao texto recebido. Nao invente fatos ou afirme ter ouvido audio ou executado acoes. Abrir programas/sites e feito fora do chat por comandos definidos; voce nao controla o computador.";
 if(historyUser.length()){JsonObject oldUser=messages.createNestedObject();oldUser["role"]="user";oldUser["content"]=historyUser;JsonObject oldAnswer=messages.createNestedObject();oldAnswer["role"]="assistant";oldAnswer["content"]=historyAnswer;}
 JsonObject user=messages.createNestedObject();user["role"]="user";user["content"]=prompt;
 String body;serializeJson(request,body);int code=http.POST(body);
 if(code!=200){char tlsError[160]={0};int tlsCode=client.lastError(tlsError,sizeof(tlsError));http.end();notice="Falha na IA. Veja USB.";render();emit("error",code==401?"Chave invalida para o provedor selecionado (401).":code==429?"Provedor recusou por limite ou capacidade (429). Nao significa necessariamente que voce gastou sua cota.":String("Falha HTTPS/API: ")+String(code)+"; TLS "+String(tlsCode)+" "+tlsError);return;}
 StaticJsonDocument<512> filter;filter["choices"][0]["message"]["content"]=true;
 DynamicJsonDocument result(8192);
 auto error=deserializeJson(result,http.getStream(),DeserializationOption::Filter(filter));
 String received=result["choices"][0]["message"]["content"].as<String>();http.end();
 if(error||!received.length()||received=="null"){notice="Resposta indisponivel";render();emit("error","Resposta vazia ou maior que o limite local.");return;}
 answer=received;historyUser=boundedUtf8(prompt,512);historyAnswer=boundedUtf8(received,1024);render();emit("reply",answer);
}
void voiceError(const String& text){answer="";page=0;notice=text;render();emit("error",text);}
bool voiceReady(){
 if(portal.active()){voiceError("Feche a configuracao: segure GPIO0 por 2 segundos.");return false;}
 if(settings.provider!="groq"||!settings.apiKey.startsWith("gsk_")){voiceError("Voz requer chave Groq. Configure pelo USB.");return false;}
 if(WiFi.status()!=WL_CONNECTED){voiceError(wifiDiagnosis());return false;}
 if(time(nullptr)<1700000000){voiceError("Aguarde o relogio HTTPS; tente novamente.");return false;}
 return true;
}
void transcribe(VoiceAudio& recording){
 feedWatchdog();
 notice="Transcrevendo...";render();emit("busy","Transcrevendo audio na Groq.");
 String text;
 {
  WiFiClientSecure client;client.setCACert(reinterpret_cast<const char*>(caRoots));client.setHandshakeTimeout(15);
  HTTPClient http;http.useHTTP10(true);http.setReuse(false);http.setConnectTimeout(15000);http.setTimeout(45000);
  if(!http.begin(client,"https://api.groq.com/openai/v1/audio/transcriptions")){voiceError("Falha ao iniciar HTTPS de voz.");return;}
  http.addHeader("Authorization","Bearer "+settings.apiKey);
  http.addHeader("Content-Type","multipart/form-data; boundary=teraVoiceBoundary");
  unsigned actualRate= recording.elapsed?lround(double(recording.count)*1000000/recording.elapsed):VoiceAudio::rate;
  VoiceMultipart payload(recording,actualRate);
  int code=http.sendRequest("POST",&payload,payload.size());
  StaticJsonDocument<384> transport;transport["event"]="audio_upload";transport["http"]=code;transport["bodyBytesRead"]=payload.size()-payload.available();transport["expectedBytes"]=payload.size();transport["freeHeap"]=ESP.getFreeHeap();serializeJson(transport,Serial);Serial.println();
  recording.clear();
  if(code!=200){char tls[160]={0};int tlsCode=client.lastError(tls,sizeof(tls));http.end();voiceError(code==401?"Chave Groq invalida (401).":code==429?"Groq: limite/capacidade (429). Tente mais tarde.":String("Falha na transcricao HTTP ")+code+(tlsCode<0?" TLS "+String(tlsCode)+" "+tls:""));return;}
  StaticJsonDocument<384> filter;filter["text"]=true;filter["segments"][0]["no_speech_prob"]=true;
  DynamicJsonDocument result(8192);
  auto err=deserializeJson(result,http.getStream(),DeserializationOption::Filter(filter));http.end();
  if(err){voiceError("Falha ao ler a transcricao.");return;}
  text=result["text"].as<String>();text.trim();
  bool speech=false;JsonArray segments=result["segments"].as<JsonArray>();
  for(JsonObject segment:segments)if(segment["no_speech_prob"].is<float>()&&segment["no_speech_prob"].as<float>()<0.6f)speech=true;
  if(!speech||!text.length()||text=="null"){voiceError("Fala nao identificada. Tente novamente.");return;}
 }
 if(text.length()>1024){voiceError("Fala longa demais. Use uma frase curta.");return;}
 emit("transcript",text);notice="Ouvi: "+text;render();dispatchInput(text);
}
void recordVoice(bool timed=false){
 if(!voiceReady())return;
 VoiceAudio recording;
 if(!recording.allocate()){voiceError("Memoria insuficiente para gravar. Reinicie.");return;}
 answer="";page=0;
 if(timed){notice="Prepare-se: gravacao em 3 segundos";render();emit("countdown","Fale em 3 segundos, durante 10 segundos.");delay(3000);}
 uint32_t centerSum=0;for(unsigned i=0;i<256;i++){centerSum+=analogRead(Board::microphone);delayMicroseconds(125);}recording.center=centerSum/256;
 notice="GRAVANDO... Solte GPIO35 para enviar. Maximo 10 segundos.";render();emit("recording");
 uint32_t start=micros(),next=start;
 unsigned releaseSamples=0;
 while(recording.count<VoiceAudio::limit){
  if(!timed){if(digitalRead(Board::talkButton)==HIGH)releaseSamples++;else releaseSamples=0;if(releaseSamples>=160)break;}
  while((int32_t)(micros()-next)<0){}
  if((int32_t)(micros()-next)>30)recording.late++;
  recording.put(recording.count++,analogRead(Board::microphone));next+=125;
  if(recording.count%8000==0&&recording.count<VoiceAudio::limit)delay(1);
 }
 recording.elapsed=micros()-start;
 if(recording.count<4000){voiceError("Gravacao curta. Segure por pelo menos meio segundo.");return;}
 recording.measure();
 StaticJsonDocument<512> metrics;metrics["event"]="audio_metrics";metrics["samples"]=recording.count;metrics["elapsedUs"]=recording.elapsed;metrics["rms"]=recording.rms;metrics["noiseRms"]=noiseRms;metrics["clipped"]=recording.clipped;metrics["pcmClipped"]=recording.pcmClipped;metrics["late"]=recording.late;metrics["freeHeap"]=ESP.getFreeHeap();serializeJson(metrics,Serial);Serial.println();
 if(recording.clipped>recording.count/100||recording.pcmClipped>recording.count/100){voiceError("Audio saturado. Reduza o ganho do amplificador ou afaste a boca.");return;}
 if(recording.rms<max(6.0,noiseRms*1.45)){voiceError("Sinal baixo ou silencio. Fale mais perto do microfone.");return;}
 recording.pack();transcribe(recording);
}
void calibrateNoise(){
 answer="";notice="Medindo ruido: fique em silencio por 1 segundo.";render();emit("busy",notice);delay(500);
 double sum=0,square=0;uint32_t next=micros();
 for(unsigned i=0;i<8000;i++){while((int32_t)(micros()-next)<0){}double v=analogRead(Board::microphone);sum+=v;square+=v*v;next+=125;}
 noiseRms=sqrt(max(0.0,square/8000-pow(sum/8000,2)));notice="Ruido calibrado. Segure GPIO35 para falar.";render();emit("network",notice);status();
}
void command(const String& line){
 DynamicJsonDocument doc(4096);if(deserializeJson(doc,line)){emit("error","JSON invalido.");return;}
 String op=doc["op"]|"";if(op=="status"){status();return;}
 if(op=="portal"){if(doc["enabled"]|true)showPortal();else{portal.stop();answer="";notice="Configuracao fechada. Segure GPIO35 para falar.";render();emit("network",notice);}return;}
 if(op=="portal_test"){if(!portal.active())showPortal();if(!portal.startSelfTest())emit("error","Teste do portal indisponivel.");return;}
 if(op=="clear_history"){historyUser="";historyAnswer="";answer="";notice="Conversa limpa.";render();emit("network",notice);return;}
 if(op=="bridge_hello"||op=="bridge_ping"){
  String session=doc["session"]|"";bool valid=session.length()==32;for(unsigned i=0;i<session.length();i++)valid=valid&&isxdigit((unsigned char)session[i]);
  if(!valid||(op=="bridge_ping"&&session!=pcSession)){emit("error","Sessao do PC invalida.");return;}
  if(op=="bridge_hello"&&session!=pcSession){pcSession=session;pcPending="";}
  pcLastSeen=millis();if(doc["actions"].is<bool>())pcActions=doc["actions"].as<bool>();if(doc["speech"].is<bool>())pcSpeech=doc["speech"].as<bool>();
  StaticJsonDocument<192> ack;ack["event"]=op=="bridge_hello"?"bridge_ready":"bridge_pong";ack["session"]=pcSession;ack["bootId"]=bootId;serializeJson(ack,Serial);Serial.println();return;
 }
 if(op=="pc_result"){
  if(!pcPending.length()||doc["session"]!=pcSession||!doc["id"].is<uint32_t>()||doc["id"].as<uint32_t>()!=pcActionId||!doc["ok"].is<bool>()){emit("error","Resultado do PC invalido.");return;}
  answer=doc["ok"].as<bool>()?"Solicitei ao PC que abrisse "+actionLabel(pcPending)+".":"Nao foi possivel abrir "+actionLabel(pcPending)+" no PC.";pcPending="";page=0;render();emit("reply",answer);return;
 }
 if(op=="voice_test"){recordVoice(true);return;}
 if(op=="calibrate_noise"){calibrateNoise();return;}
 if(op=="voice_fixture"){
  // USB-only diagnostic: prerecorded PCM8, never presented as a live capture.
  if(!voiceReady())return;
  unsigned count=doc["samples"]|0U;
  if(count<4000||count>VoiceAudio::limit||!doc["crc"].is<uint32_t>()){emit("error","Fixture invalida.");return;}
  VoiceAudio audio;if(!audio.allocate()){emit("error","Memoria insuficiente.");return;}
  audio.count=count;audio.elapsed=count*125;emit("fixture_ready","Teste com audio previamente gravado, nao captacao ao vivo.");
  uint32_t crc=0xffffffff,start=millis();unsigned received=0;
  while(received<count&&millis()-start<20000){
   if(!Serial.available()){delay(1);continue;}
   uint8_t value=Serial.read();audio.data[received++]=value;crc^=value;
   for(unsigned bit=0;bit<8;bit++)crc=(crc>>1)^((crc&1)?0xedb88320U:0);
  }
  if(received!=count||(crc^0xffffffffU)!=doc["crc"].as<uint32_t>()){voiceError("Fixture incompleta ou CRC invalido. Nao enviada.");return;}
  answer="";page=0;transcribe(audio);return;
 }
 if(op=="audio_selftest"){
  VoiceAudio audio;if(!audio.allocate()){emit("error","Falha de memoria no autoteste.");return;}
  audio.count=VoiceAudio::limit;audio.center=1010;for(unsigned i=0;i<audio.count;i++)audio.put(i,i%2?1020:1000);
  audio.measure();bool ok=abs(audio.mean-1010)<0.01&&abs(audio.rms-10)<0.01;audio.pack();
  for(unsigned i=0;i<audio.count;i++)if(audio.pcm(i)!=(i%2?242:14))ok=false;
  VoiceMultipart stream(audio,8000);size_t n=0;while(stream.read()>=0)n++;
  ok=ok&&n==stream.size()&&stream.available()==0;
  unsigned heap=ESP.getFreeHeap(),largest=ESP.getMaxAllocHeap();
  StaticJsonDocument<768> result;result["event"]="audio_selftest";result["passed"]=ok;result["samples"]=80000;result["payloadBytes"]=n;result["heapBeforeTLS"]=heap;result["largestBlockBeforeTLS"]=largest;
  if(WiFi.status()==WL_CONNECTED&&time(nullptr)>=1700000000){
   WiFiClientSecure client;client.setCACert(reinterpret_cast<const char*>(caRoots));client.setHandshakeTimeout(15);
   HTTPClient http;http.useHTTP10(true);http.setConnectTimeout(15000);http.setTimeout(15000);
   http.begin(client,"https://api.groq.com/openai/v1/models");int code=http.GET();char tls[160]={0};int tlsCode=client.lastError(tls,sizeof(tls));
   result["tlsWithAudioBuffer"]=code==401;result["http"]=code;result["tlsCode"]=tlsCode;if(code!=401)result["tlsError"]=tls;http.end();
  }
  audio.clear();serializeJson(result,Serial);Serial.println();return;
 }
 if(op=="probe_groq"||op=="probe_groq_auth"){
  if(WiFi.status()!=WL_CONNECTED||time(nullptr)<1700000000){emit("error","Aguarde WiFi e relogio.");return;}
  WiFiClientSecure client;client.setCACert(reinterpret_cast<const char*>(caRoots));client.setHandshakeTimeout(15);
  HTTPClient http;http.useHTTP10(true);http.setConnectTimeout(15000);http.setTimeout(15000);
  http.begin(client,"https://api.groq.com/openai/v1/models");
  if(op=="probe_groq_auth"){if(settings.provider!="groq"||!settings.apiKey.length()){http.end();emit("error","Configure Groq primeiro.");return;}http.addHeader("Authorization","Bearer "+settings.apiKey);}
  int code=http.GET();http.end();
  emit("network",String(op=="probe_groq_auth"?"Teste de autenticacao Groq: HTTP ":"Teste HTTPS Groq sem chave: HTTP ")+String(code)+(op=="probe_groq"&&code==401?" (TLS OK; falta chave Groq).":""));return;
 }
 if(op=="scan"){
  WiFi.disconnect(false,false);delay(150);
  int count=WiFi.scanNetworks();bool found=false;
  for(int i=0;i<count;i++)if(WiFi.SSID(i)==settings.wifiSsid)found=true;
  WiFi.scanDelete();
  emit("network",count<0?"Nao foi possivel procurar redes. Tente novamente.":found?"A rede configurada foi encontrada em 2.4 GHz. Confira senha/seguranca se nao conectar.":"A rede configurada nao apareceu na busca 2.4 GHz. Confira o nome exato e aproxime a placa do roteador.");connectWifi();return;
 }
 if(op=="configure"){
  String error;if(!applyConfiguration(doc.as<JsonObjectConst>(),false,error)){emit("error",error);return;}
  emit("configured","Configuracao salva; conectando WiFi.");return;
 }
 if(op=="chat"){
  if(!doc["text"].is<String>()){emit("error","Texto ausente.");return;}
  String text=doc["text"];text.trim();if(text.length()<1||text.length()>1024){emit("error","Envie de 1 a 1024 bytes de texto.");return;}dispatchInput(text);return;
 }
 emit("error","Comando desconhecido.");
}
void setup(){
 bootId=esp_random();
 Serial.setRxBufferSize(4096);Serial.begin(115200);Serial.setDebugOutput(false);pinMode(Board::menuButton,INPUT_PULLUP);pinMode(Board::talkButton,INPUT);pinMode(Board::microphone,INPUT);analogReadResolution(12);analogSetPinAttenuation(Board::microphone,ADC_11db);
 WiFi.onEvent([](WiFiEvent_t event,WiFiEventInfo_t info){if(event==ARDUINO_EVENT_WIFI_STA_DISCONNECTED)disconnectReason=info.wifi_sta_disconnected.reason;});
 settings.load();screen.init();screen.setRotation(1);pinMode(Board::displayBacklight,OUTPUT);digitalWrite(Board::displayBacklight,HIGH);
 watchdogEnabled=esp_task_wdt_init(120,true)==ESP_OK&&esp_task_wdt_add(nullptr)==ESP_OK;
 input.reserve(3072);connectWifi();if(!settings.wifiSsid.length()){portalAttempted=true;showPortal();}render();status();
}
void loop(){
 feedWatchdog();
 while(Serial.available()){
  char c=Serial.read();if(c=='\n'){if(dropping)emit("error","Comando excede 3072 bytes.");else if(input.length())command(input);input="";dropping=false;}
  else if(c!='\r'&&!dropping){if(input.length()>=3072){dropping=true;input="";}else input+=c;}
 }
 bool pressed=digitalRead(Board::menuButton)==LOW;
 if(pressed!=lastButton&&millis()-lastPress>40){lastPress=millis();lastButton=pressed;if(pressed){menuStarted=millis();menuLong=false;}else if(!menuLong){page++;render();}}
 if(pressed&&!menuLong&&millis()-menuStarted>=1800){menuLong=true;if(portal.active()){portal.stop();answer="";notice="Configuracao fechada.";render();}else showPortal();}
 if(digitalRead(Board::talkButton)==HIGH)talkArmed=true;
 else if(talkArmed){delay(25);if(digitalRead(Board::talkButton)==LOW){talkArmed=false;recordVoice();}}
 bool hadPortal=portal.active();portal.poll();if(hadPortal&&!portal.active()){answer="";notice="Configuracao encerrada apos 10 minutos.";render();}
 String portalReport=portal.takeTestResult();if(portalReport.length())Serial.println(portalReport);
 if(reconnectAt&&(int32_t)(millis()-reconnectAt)>=0){reconnectAt=0;portal.stop();WiFi.disconnect(false,false);connectWifi();render();}
 if(pcPending.length()&&(int32_t)(millis()-pcDeadline)>=0){pcPending="";voiceError("O aplicativo do PC nao respondeu. Abra Tera no computador.");}
 static bool connected=false;bool now=WiFi.status()==WL_CONNECTED;if(now!=connected){connected=now;if(now&&!answer.length()&&!portal.active())notice="WiFi conectado. Segure GPIO35 para falar (Groq).";render();status();}delay(5);
 if(!now&&!portal.active()&&settings.wifiSsid.length()&&!wifiReported&&millis()-wifiStarted>=20000){wifiReported=true;notice=wifiDiagnosis();render();status();}
 if(!now&&!portalAttempted&&millis()-wifiStarted>=30000){portalAttempted=true;showPortal();}
}
