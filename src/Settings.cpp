#include "Settings.h"
#include <Preferences.h>
#include <ArduinoJson.h>

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
    }
    storage.end();
    return;
  }
  wifiSsid = storage.getString("ssid", "");
  wifiPassword = storage.getString("wifi_pass", "");
  apiKey = storage.getString("api_key", "");
  provider = storage.getString("provider", "openrouter");
  storage.end();
}

bool Settings::valid(String& error) const {
  if (provider != "groq" && provider != "openrouter") { error="Provedor nao permitido."; return false; }
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
  if (doc.containsKey("model") && doc["model"]!="openrouter/free") { error="Modelo personalizado nao permitido.";return false; }
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
  String payload;
  serializeJson(doc, payload);
  Preferences storage;
  if (!storage.begin("tera", false)) return false;
  bool ok = storage.putString("config", payload) == payload.length();
  storage.end();
  return ok;
}
