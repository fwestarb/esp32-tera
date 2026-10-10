#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <functional>
#include <freertos/queue.h>
#include "Settings.h"

class Portal {
 public:
  using Save=std::function<bool(JsonObjectConst,String&)>;
  Portal(Settings& settings,Save save):settings(settings),save(save){}
  bool active()const{return running;}
  bool start();
  void stop();
  void poll();
  const String& name()const{return ssid;}
  const String& password()const{return pass;}
  bool startSelfTest();
  String takeTestResult();
 private:
  Settings& settings;Save save;
  WebServer server{80};DNSServer dns;
  bool running=false,registered=false;
  uint32_t started=0;
  String ssid,pass,token;
  QueueHandle_t testQueue=nullptr;
  bool testRunning=false;
  bool fromAP();
  bool protectedRequest();
  void routes();
  void json(int code,const String& body);
  static void testTask(void* arg);
};
