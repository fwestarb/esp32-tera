#include <Arduino.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <time.h>
#include "Board.h"
#include "Settings.h"
#include "VoiceAudio.h"
// Certificate verification plus the voice/command call chain exceeds 8 KB.
SET_LOOP_TASK_STACK_SIZE(16384);
extern const uint8_t caRoots[] asm("_binary_data_cert_openrouter_roots_pem_start");
TFT_eSPI screen;
Settings settings;
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
 DynamicJsonDocument doc(6144);doc["event"]=type;if(message.length())doc["message"]=message;serializeJson(doc,Serial);Serial.println();
}
void status(){
 StaticJsonDocument<1024> doc;doc["event"]="status";doc["version"]="0.4.0";
 doc["networkMessage"]=wifiDiagnosis();doc["disconnectReason"]=disconnectReason;
 doc["configured"]=settings.wifiSsid.length()>0&&settings.apiKey.length()>0;doc["wifi"]=WiFi.status()==WL_CONNECTED;
 doc["freeHeap"]=ESP.getFreeHeap();doc["microphone"]=true;doc["voiceReady"]=settings.provider=="groq"&&settings.apiKey.length()>0&&WiFi.status()==WL_CONNECTED&&time(nullptr)>=1700000000;doc["noiseRms"]=noiseRms;doc["keyboard"]=false;doc["speaker"]=false;doc["provider"]=settings.provider;doc["model"]=settings.provider=="groq"?"openai/gpt-oss-20b":"google/gemma-4-26b-a4b-it:free";
 doc["stackWatermark"]=uxTaskGetStackHighWaterMark(nullptr);
 serializeJson(doc,Serial);Serial.println();
}
void connectWifi(){
 wifiStarted=millis();wifiReported=false;disconnectReason=0;
 WiFi.mode(WIFI_STA);if(settings.wifiSsid.length()){WiFi.begin(settings.wifiSsid.c_str(),settings.wifiPassword.c_str());configTime(0,0,"pool.ntp.org","time.google.com");notice="Conectando WiFi. Depois segure GPIO35 para falar.";}
}
void chat(const String& prompt){
 if(!settings.apiKey.length()){emit("error","Configure sua chave primeiro.");return;}
 if(WiFi.status()!=WL_CONNECTED){emit("error",wifiDiagnosis());return;}
 if(time(nullptr)<1700000000){emit("error","Aguarde a sincronizacao do relogio para HTTPS.");return;}
 answer="";page=0;notice="Consultando IA...";render();emit("busy");
 WiFiClientSecure client;client.setCACert(reinterpret_cast<const char*>(caRoots));client.setHandshakeTimeout(15);
 HTTPClient http;http.useHTTP10(true);http.setReuse(false);http.setConnectTimeout(15000);http.setTimeout(45000);
 if(!http.begin(client,settings.provider=="groq"?"https://api.groq.com/openai/v1/chat/completions":"https://openrouter.ai/api/v1/chat/completions")){emit("error","Falha ao iniciar HTTPS.");return;}
 http.addHeader("Authorization","Bearer "+settings.apiKey);http.addHeader("Content-Type","application/json");
 const bool groq=settings.provider=="groq";
 DynamicJsonDocument request(3072);request["model"]=groq?"openai/gpt-oss-20b":"google/gemma-4-26b-a4b-it:free";request["max_tokens"]=groq?1024:320;
 if(groq){request["reasoning_effort"]="low";request["include_reasoning"]=false;}
 JsonArray messages=request.createNestedArray("messages");
 JsonObject system=messages.createNestedObject();system["role"]="system";system["content"]="Voce e Tera. Responda em portugues de forma breve e clara. Nao invente fatos.";
 JsonObject user=messages.createNestedObject();user["role"]="user";user["content"]=prompt;
 String body;serializeJson(request,body);int code=http.POST(body);
 if(code!=200){char tlsError[160]={0};int tlsCode=client.lastError(tlsError,sizeof(tlsError));http.end();notice="Falha na IA. Veja USB.";render();emit("error",code==401?"Chave invalida para o provedor selecionado (401).":code==429?"Provedor recusou por limite ou capacidade (429). Nao significa necessariamente que voce gastou sua cota.":String("Falha HTTPS/API: ")+String(code)+"; TLS "+String(tlsCode)+" "+tlsError);return;}
 StaticJsonDocument<512> filter;filter["choices"][0]["message"]["content"]=true;
 DynamicJsonDocument result(8192);
 auto error=deserializeJson(result,http.getStream(),DeserializationOption::Filter(filter));
 String received=result["choices"][0]["message"]["content"].as<String>();http.end();
 if(error||!received.length()||received=="null"){notice="Resposta indisponivel";render();emit("error","Resposta vazia ou maior que o limite local.");return;}
 answer=received;render();emit("reply",answer);
}
void voiceError(const String& text){answer="";page=0;notice=text;render();emit("error",text);}
bool voiceReady(){
 if(settings.provider!="groq"||!settings.apiKey.startsWith("gsk_")){voiceError("Voz requer chave Groq. Configure pelo USB.");return false;}
 if(WiFi.status()!=WL_CONNECTED){voiceError(wifiDiagnosis());return false;}
 if(time(nullptr)<1700000000){voiceError("Aguarde o relogio HTTPS; tente novamente.");return false;}
 return true;
}
void transcribe(VoiceAudio& recording){
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
 emit("transcript",text);notice="Ouvi: "+text;render();delay(1200);chat(text);
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
  if(!doc["ssid"].is<String>()||!doc["password"].is<String>()||!doc["key"].is<String>()){emit("error","Configuracao incompleta.");return;}
  String ssid=doc["ssid"],pass=doc["password"],key=doc["key"];
  String provider=doc["provider"]|"openrouter";
  if(provider!="groq"&&provider!="openrouter"){emit("error","Provedor nao permitido.");return;}
  for(unsigned i=0;i<key.length();i++){char c=key[i];if(!isalnum(static_cast<unsigned char>(c))&&c!='-'&&c!='_'){emit("error","Formato da chave invalido.");return;}}
  if(ssid.length()<1||ssid.length()>32||pass.length()>63||(pass.length()>0&&pass.length()<8)||key.length()<20||key.length()>256||!key.startsWith(provider=="groq"?"gsk_":"sk-or-")){emit("error","Confira SSID, senha WiFi e chave do provedor selecionado.");return;}
  if(doc.containsKey("model")&&doc["model"]!="openrouter/free"){emit("error","Somente openrouter/free permitido nesta etapa.");return;}
  Settings next;next.wifiSsid=ssid;next.wifiPassword=pass;next.apiKey=key;next.provider=provider;
  if(!next.save()){emit("error","Falha ao salvar na placa.");return;}
  settings=next;WiFi.disconnect();connectWifi();render();emit("configured","Configuracao salva; conectando WiFi.");return;
 }
 if(op=="chat"){
  if(!doc["text"].is<String>()){emit("error","Texto ausente.");return;}
  String text=doc["text"];text.trim();if(text.length()<1||text.length()>1024){emit("error","Envie de 1 a 1024 bytes de texto.");return;}chat(text);return;
 }
 emit("error","Comando desconhecido.");
}
void setup(){
 Serial.begin(115200);Serial.setDebugOutput(false);pinMode(Board::menuButton,INPUT_PULLUP);pinMode(Board::talkButton,INPUT);pinMode(Board::microphone,INPUT);analogReadResolution(12);analogSetPinAttenuation(Board::microphone,ADC_11db);
 WiFi.onEvent([](WiFiEvent_t event,WiFiEventInfo_t info){if(event==ARDUINO_EVENT_WIFI_STA_DISCONNECTED)disconnectReason=info.wifi_sta_disconnected.reason;});
 settings.load();screen.init();screen.setRotation(1);pinMode(Board::displayBacklight,OUTPUT);digitalWrite(Board::displayBacklight,HIGH);
 input.reserve(3072);connectWifi();render();status();
}
void loop(){
 while(Serial.available()){
  char c=Serial.read();if(c=='\n'){if(dropping)emit("error","Comando excede 3072 bytes.");else if(input.length())command(input);input="";dropping=false;}
  else if(c!='\r'&&!dropping){if(input.length()>=3072){dropping=true;input="";}else input+=c;}
 }
 bool pressed=digitalRead(Board::menuButton)==LOW;
 if(pressed!=lastButton&&millis()-lastPress>40){lastPress=millis();lastButton=pressed;if(pressed){page++;render();}}
 if(digitalRead(Board::talkButton)==HIGH)talkArmed=true;
 else if(talkArmed){delay(25);if(digitalRead(Board::talkButton)==LOW){talkArmed=false;recordVoice();}}
 static bool connected=false;bool now=WiFi.status()==WL_CONNECTED;if(now!=connected){connected=now;if(now&&!answer.length())notice="WiFi conectado. Segure GPIO35 para falar (Groq).";render();status();}delay(5);
 if(!now&&settings.wifiSsid.length()&&!wifiReported&&millis()-wifiStarted>=20000){wifiReported=true;notice=wifiDiagnosis();render();status();}
}
