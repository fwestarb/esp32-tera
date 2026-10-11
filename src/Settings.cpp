#include "Settings.h"
#include <Preferences.h>
#include <ArduinoJson.h>

String Settings::defaultModel(const String& provider) {
  return provider=="groq"?"openai/gpt-oss-120b":"openrouter/free";
}
bool Settings::allowedModel(const String& provider,const String& model) {
  if(provider=="groq")return model=="openai/gpt-oss-120b"||model=="openai/gpt-oss-20b";
  // Only the free router or explicitly free model identifiers are accepted.
  if(provider!="openrouter"||model.length()>128)return false;
  if(model=="openrouter/free")return true;
  if(!model.endsWith(":free")||model.indexOf('/')<1)return false;
  for(unsigned i=0;i<model.length();i++)if(!isalnum((unsigned char)model[i])&&model[i]!='/'&&model[i]!='-'&&model[i]!='_'&&model[i]!='.'&&model[i]!=':')return false;
  return true;
}

void Settings::load() {
  Preferences storage;
  if (!storage.begin("tera", true)) return; // First boot has no configuration.
  String saved = storage.getString("config", "");
  if (saved.length()) {
    DynamicJsonDocument doc(2048);
    if (!deserializeJson(doc, saved)) {
      wifiSsid = doc["ssid"].as<String>();
      wifiPassword = doc["password"].as<String>();
      apiKey = doc["key"].as<String>();
      provider = doc["provider"] | "openrouter";
      model = doc["model"] | defaultModel(provider);
    }
    storage.end();
    return;
  }
  wifiSsid = storage.getString("ssid", "");
  wifiPassword = storage.getString("wifi_pass", "");
  apiKey = storage.getString("api_key", "");
  provider = storage.getString("provider", "openrouter");
  model = defaultModel(provider);
  storage.end();
}

bool Settings::valid(String& error) const {
  if (provider != "groq" && provider != "openrouter") { error="Provedor nao permitido."; return false; }
  if (!allowedModel(provider,model)) { error="Modelo nao permitido: selecione GPT-OSS 120B/20B na Groq ou um modelo :free no OpenRouter.";return false; }
  if (wifiSsid.length()<1 || wifiSsid.length()>32 || wifiPassword.length()>63 || (wifiPassword.length()>0 && wifiPassword.length()<8)) {
    error="Confira o nome da rede (1-32 bytes) e a senha (8-63 caracteres, ou rede aberta).";return false;
  }
  if (apiKey.length()<20 || apiKey.length()>256 || !apiKey.startsWith(provider=="groq"?"gsk_":"sk-or-")) {
    error="Informe uma chave valida para o provedor selecionado.";return false;
  }
  for (unsigned i=0;i<apiKey.length();i++) if (!isalnum((unsigned char)apiKey[i]) && apiKey[i]!='-' && apiKey[i]!='_') {
    error="Formato da chave invalido.";return false;
  }
  return true;
}

bool Settings::parse(JsonObjectConst doc, const Settings& current, bool keepBlank, Settings& next, String& error) {
  if (!doc["ssid"].is<String>() || !doc["password"].is<String>() || !doc["key"].is<String>()) {
    error="Configuracao incompleta.";return false;
  }
  next.wifiSsid=doc["ssid"].as<String>();next.wifiPassword=doc["password"].as<String>();next.apiKey=doc["key"].as<String>();
  next.provider=doc["provider"] | (keepBlank?current.provider.c_str():"openrouter");
  next.model=doc["model"] | ((keepBlank&&next.provider==current.provider)?current.model:defaultModel(next.provider));
  if(doc.containsKey("model")&&!doc["model"].is<String>()){error="Modelo invalido.";return false;}
  if (keepBlank) {
    if (!next.wifiPassword.length() && next.wifiSsid==current.wifiSsid && !(doc["openNetwork"]|false)) next.wifiPassword=current.wifiPassword;
    if (!next.apiKey.length() && next.provider==current.provider) next.apiKey=current.apiKey;
  }
  return next.valid(error);
}

bool Settings::save() {
  DynamicJsonDocument doc(2048);
  doc["ssid"] = wifiSsid;
  doc["password"] = wifiPassword;
  doc["key"] = apiKey;
  doc["provider"] = provider;
  doc["model"] = model;
  String payload;
  serializeJson(doc, payload);
  Preferences storage;
  if (!storage.begin("tera", false)) return false;
  bool ok = storage.putString("config", payload) == payload.length();
  storage.end();
  return ok;
}
