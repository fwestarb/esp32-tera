#include "Portal.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <esp_system.h>
extern const uint8_t portalPage[] asm("_binary_data_portal_html_start");

namespace {
String hexRandom(){char value[17];snprintf(value,sizeof(value),"%08lx%08lx",(unsigned long)esp_random(),(unsigned long)esp_random());return value;}
String escapeHtml(String value){value.replace("&","&amp;");value.replace("<","&lt;");value.replace(">","&gt;");value.replace("\"","&quot;");value.replace("'","&#39;");return value;}
}

bool Portal::start(){
 if(running)return true;
 if(!registered)routes();
 char suffix[5];snprintf(suffix,sizeof(suffix),"%04X",(unsigned)(ESP.getEfuseMac()>>32)&0xffff);
 ssid="TERA-"+String(suffix);pass=hexRandom();token=hexRandom()+hexRandom();
 WiFi.mode(WIFI_AP_STA);
 WiFi.softAPConfig(IPAddress(192,168,4,1),IPAddress(192,168,4,1),IPAddress(255,255,255,0));
 if(!WiFi.softAP(ssid.c_str(),pass.c_str(),1,false,2))return false;
 dns.start(53,"*",WiFi.softAPIP());server.begin();started=millis();running=true;return true;
}
void Portal::stop(){
 if(!running)return;
 dns.stop();server.stop();WiFi.softAPdisconnect(true);WiFi.mode(WIFI_STA);running=false;pass="";token="";
}
void Portal::poll(){if(!running)return;dns.processNextRequest();server.handleClient();if(millis()-started>=600000&&!testRunning)stop();}
bool Portal::fromAP(){return running&&server.client().localIP()==WiFi.softAPIP();}
void Portal::json(int code,const String& body){server.sendHeader("Cache-Control","no-store");server.sendHeader("X-Content-Type-Options","nosniff");server.send(code,"application/json; charset=utf-8",body);}
bool Portal::protectedRequest(){
 if(!fromAP()){json(403,"{\"error\":\"Use a rede Wi-Fi da Tera.\"}");return false;}
 if(server.hostHeader()!="192.168.4.1" || server.header("X-Tera-Token")!=token){json(403,"{\"error\":\"Sessao invalida. Abra novamente a pagina.\"}");return false;}
 String origin=server.header("Origin");
 if(origin.length()&&origin!="http://192.168.4.1"){json(403,"{\"error\":\"Origem invalida.\"}");return false;}
 return true;
}
void Portal::routes(){
 const char* headers[]={"X-Tera-Token","Origin"};server.collectHeaders(headers,2);
 server.on("/",HTTP_GET,[this](){
  if(!fromAP()){json(403,"{\"error\":\"Use a rede Wi-Fi da Tera.\"}");return;}
  if(server.hostHeader()!="192.168.4.1"){server.sendHeader("Location","http://192.168.4.1/");server.send(302);return;}
  String page((const char*)portalPage);page.replace("{{TOKEN}}",token);page.replace("{{MODEL}}",escapeHtml(settings.model));page.replace("{{SSID}}",escapeHtml(settings.wifiSsid));page.replace("{{GROQ_SELECTED}}",settings.provider=="groq"?"selected":"");page.replace("{{OR_SELECTED}}",settings.provider=="openrouter"?"selected":"");
  server.sendHeader("Cache-Control","no-store");server.sendHeader("X-Frame-Options","DENY");
  server.sendHeader("Content-Security-Policy","default-src 'none'; style-src 'unsafe-inline'; script-src 'nonce-"+token+"'; connect-src 'self'; img-src 'self'; frame-ancestors 'none'; base-uri 'none'; form-action 'self'");
  server.send(200,"text/html; charset=utf-8",page);
 });
 server.on("/api/status",HTTP_GET,[this](){
  if(!protectedRequest())return;
  StaticJsonDocument<512> doc;doc["wifi"]=WiFi.status()==WL_CONNECTED;doc["configured"]=settings.apiKey.length()>0;doc["provider"]=settings.provider;doc["model"]=settings.model;doc["expiresSeconds"]=max(0L,600L-long((millis()-started)/1000));
  String body;serializeJson(doc,body);json(200,body);
 });
 server.on("/api/config",HTTP_POST,[this](){
  if(!protectedRequest())return;
  String body=server.arg("plain");if(body.length()>2048){json(413,"{\"error\":\"Configuracao muito grande.\"}");return;}
  DynamicJsonDocument doc(2048);if(deserializeJson(doc,body)||!doc.is<JsonObject>()){json(400,"{\"error\":\"JSON invalido.\"}");return;}
  String error;if(!save(doc.as<JsonObjectConst>(),error)){StaticJsonDocument<384> response;response["error"]=error;String payload;serializeJson(response,payload);json(400,payload);return;}
  json(200,"{\"ok\":true,\"message\":\"Salvo. A rede Tera sera fechada e a placa tentara conectar ao seu Wi-Fi.\"}");
 });
 server.onNotFound([this](){if(!fromAP()){json(403,"{}");return;}if(server.method()!=HTTP_GET){json(405,"{}");return;}server.sendHeader("Location","http://192.168.4.1/");server.send(302);});
 registered=true;
}

bool Portal::startSelfTest(){
 if(!running||testRunning)return false;
 if(!testQueue)testQueue=xQueueCreate(1,sizeof(char*));
 if(!testQueue)return false;
 testRunning=true;
 if(xTaskCreate(testTask,"portal-test",8192,this,1,nullptr)!=pdPASS){testRunning=false;return false;}
 return true;
}
String Portal::takeTestResult(){
 if(!testQueue)return "";char* result=nullptr;
 if(xQueueReceive(testQueue,&result,0)!=pdTRUE)return "";
 String value(result);free(result);testRunning=false;return value;
}
void Portal::testTask(void* arg){
 auto* portal=(Portal*)arg;
 DynamicJsonDocument report(2048);report["event"]="portal_test";JsonArray cases=report.createNestedArray("cases");bool passed=true;
 auto request=[&](const char* label,const String& path,const String& body,int expected,bool authorize=true){
  WiFiClient client;HTTPClient http;http.setConnectTimeout(3000);http.setTimeout(5000);http.begin(client,"http://192.168.4.1"+path);
  if(authorize)http.addHeader("X-Tera-Token",portal->token);
  int code;if(body.length()){http.addHeader("Content-Type","application/json");code=http.POST(body);}else code=http.GET();
  String response=http.getString();http.end();
  bool ok=code==expected;
  if(String(label)=="page")ok=ok&&response.indexOf("ESP32 Tera")>=0&&response.indexOf("{{TOKEN}}")<0&&(!portal->settings.apiKey.length()||response.indexOf(portal->settings.apiKey)<0)&&(!portal->settings.wifiPassword.length()||response.indexOf(portal->settings.wifiPassword)<0);
  if(String(label)=="status")ok=ok&&response.indexOf("\"key\"")<0&&response.indexOf("\"password\"")<0;
  JsonObject item=cases.createNestedObject();item["test"]=label;item["http"]=code;item["passed"]=ok;passed=passed&&ok;
 };
 request("page","/","",200);
 request("missing token","/api/status","",403,false);
 request("status","/api/status","",200);
 request("invalid JSON","/api/config","{",400);
 request("invalid configuration","/api/config","{\"ssid\":\"test\",\"password\":\"short\",\"key\":\"bad\",\"provider\":\"groq\"}",400);
 String oversized;oversized.reserve(2201);for(unsigned i=0;i<2200;i++)oversized+='x';
 request("oversize configuration","/api/config",oversized,413);
 request("unknown page redirect","/missing","",302);
 // Exercise the real DNS server without needing a second Wi-Fi device.
 WiFiUDP udp;udp.begin(0);
 const uint8_t query[]={0x54,0x52,1,0,0,1,0,0,0,0,0,0,4,'t','e','r','a',4,'t','e','s','t',0,0,1,0,1};
 udp.beginPacket(IPAddress(192,168,4,1),53);udp.write(query,sizeof(query));udp.endPacket();
 uint32_t until=millis()+3000;int size=0;while(!size&&millis()<until){size=udp.parsePacket();vTaskDelay(pdMS_TO_TICKS(5));}
 uint8_t response[128]={0};int bytes=udp.read(response,sizeof(response));udp.stop();
 bool dnsOK=bytes>=16&&response[0]==0x54&&response[1]==0x52&&response[bytes-4]==192&&response[bytes-3]==168&&response[bytes-2]==4&&response[bytes-1]==1;
 JsonObject dnsCase=cases.createNestedObject();dnsCase["test"]="captive DNS";dnsCase["passed"]=dnsOK;passed=passed&&dnsOK;
 // A valid no-op save checks the complete endpoint without changing credentials.
 String beforeSsid=portal->settings.wifiSsid,beforePass=portal->settings.wifiPassword,beforeKey=portal->settings.apiKey,beforeProvider=portal->settings.provider;
 StaticJsonDocument<512> config;config["ssid"]=beforeSsid;config["password"]="";config["key"]="";config["provider"]=beforeProvider;
 String configBody;serializeJson(config,configBody);
 request("save existing configuration","/api/config",configBody,200);
 bool unchanged=portal->settings.wifiSsid==beforeSsid&&portal->settings.wifiPassword==beforePass&&portal->settings.apiKey==beforeKey&&portal->settings.provider==beforeProvider;
 report["credentialsPreserved"]=unchanged;passed=passed&&unchanged;
 report["passed"]=passed;report["apActive"]=portal->active();
 String text;serializeJson(report,text);char* result=strdup(text.c_str());if(result)xQueueSend(portal->testQueue,&result,portMAX_DELAY);
 vTaskDelete(nullptr);
}
