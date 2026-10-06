#include <Arduino.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <time.h>
#include "Board.h"
#include "Settings.h"
extern const uint8_t caBundle[] asm("_binary_data_cert_x509_crt_bundle_bin_start");
TFT_eSPI screen;
Settings settings;
String input, answer, notice = "Configure pelo USB";
unsigned page=0;
bool dropping=false,lastButton=false;
uint32_t lastPress=0;
volatile uint8_t disconnectReason=0;
uint32_t wifiStarted=0;
bool wifiReported=false;
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
 screen.setTextColor(moss,bg);screen.drawString(String(WiFi.status()==WL_CONNECTED?"WiFi OK":"WiFi pendente")+" | "+String(page+1)+"/"+String(pages)+" GPIO0: pagina",8,119,1);
}
void emit(const char* type,const String& message=""){
 DynamicJsonDocument doc(6144);doc["event"]=type;if(message.length())doc["message"]=message;serializeJson(doc,Serial);Serial.println();
}
void status(){
 StaticJsonDocument<768> doc;doc["event"]="status";doc["version"]="0.2.1";
 doc["networkMessage"]=wifiDiagnosis();doc["disconnectReason"]=disconnectReason;
 doc["configured"]=settings.wifiSsid.length()>0&&settings.apiKey.length()>0;doc["wifi"]=WiFi.status()==WL_CONNECTED;
 doc["freeHeap"]=ESP.getFreeHeap();doc["microphone"]=false;doc["keyboard"]=false;doc["speaker"]=false;doc["model"]="openrouter/free";
 serializeJson(doc,Serial);Serial.println();
}
void connectWifi(){
 wifiStarted=millis();wifiReported=false;disconnectReason=0;
 WiFi.mode(WIFI_STA);if(settings.wifiSsid.length()){WiFi.begin(settings.wifiSsid.c_str(),settings.wifiPassword.c_str());configTime(0,0,"pool.ntp.org","time.google.com");notice="Conectando WiFi. Envie texto pelo USB.";}
}
void chat(const String& prompt){
 if(!settings.apiKey.length()){emit("error","Configure sua chave primeiro.");return;}
 if(WiFi.status()!=WL_CONNECTED){emit("error",wifiDiagnosis());return;}
 if(time(nullptr)<1700000000){emit("error","Aguarde a sincronizacao do relogio para HTTPS.");return;}
 answer="";page=0;notice="Consultando IA...";render();emit("busy");
 WiFiClientSecure client;client.setCACertBundle(caBundle);client.setHandshakeTimeout(15);
 HTTPClient http;http.useHTTP10(true);http.setReuse(false);http.setConnectTimeout(15000);http.setTimeout(45000);
 if(!http.begin(client,"https://openrouter.ai/api/v1/chat/completions")){emit("error","Falha ao iniciar HTTPS.");return;}
 http.addHeader("Authorization","Bearer "+settings.apiKey);http.addHeader("Content-Type","application/json");
 DynamicJsonDocument request(3072);request["model"]="openrouter/free";request["max_tokens"]=320;
 JsonArray messages=request.createNestedArray("messages");
 JsonObject system=messages.createNestedObject();system["role"]="system";system["content"]="Voce e Tera. Responda em portugues de forma breve e clara. Nao invente fatos.";
 JsonObject user=messages.createNestedObject();user["role"]="user";user["content"]=prompt;
 String body;serializeJson(request,body);int code=http.POST(body);
 if(code!=200){http.end();notice="Falha na IA. Veja USB.";render();emit("error",code==401?"Chave invalida (401).":code==429?"Limite gratuito atingido (429).":String("Falha HTTPS/API: ")+String(code));return;}
 StaticJsonDocument<512> filter;filter["choices"][0]["message"]["content"]=true;
 DynamicJsonDocument result(8192);
 auto error=deserializeJson(result,http.getStream(),DeserializationOption::Filter(filter));
 String received=result["choices"][0]["message"]["content"].as<String>();http.end();
 if(error||!received.length()||received=="null"){notice="Resposta indisponivel";render();emit("error","Resposta vazia ou maior que o limite local.");return;}
 answer=received;render();emit("reply",answer);
}
void command(const String& line){
 DynamicJsonDocument doc(4096);if(deserializeJson(doc,line)){emit("error","JSON invalido.");return;}
 String op=doc["op"]|"";if(op=="status"){status();return;}
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
  for(unsigned i=0;i<key.length();i++){char c=key[i];if(!isalnum(static_cast<unsigned char>(c))&&c!='-'&&c!='_'){emit("error","Formato da chave invalido.");return;}}
  if(ssid.length()<1||ssid.length()>32||pass.length()>63||(pass.length()>0&&pass.length()<8)||key.length()<20||key.length()>256||!key.startsWith("sk-or-")){emit("error","Confira SSID, senha WiFi e chave OpenRouter.");return;}
  if(doc.containsKey("model")&&doc["model"]!="openrouter/free"){emit("error","Somente openrouter/free permitido nesta etapa.");return;}
  Settings next;next.wifiSsid=ssid;next.wifiPassword=pass;next.apiKey=key;
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
 Serial.begin(115200);Serial.setDebugOutput(false);pinMode(Board::menuButton,INPUT_PULLUP);
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
 static bool connected=false;bool now=WiFi.status()==WL_CONNECTED;if(now!=connected){connected=now;render();status();}delay(5);
 if(!now&&settings.wifiSsid.length()&&!wifiReported&&millis()-wifiStarted>=20000){wifiReported=true;notice=wifiDiagnosis();render();status();}
}
