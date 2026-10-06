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
      textModel = "openrouter/free";
    }
    storage.end();
    return;
  }
  wifiSsid = storage.getString("ssid", "");
  wifiPassword = storage.getString("wifi_pass", "");
  apiKey = storage.getString("api_key", "");
  textModel = storage.getString("text_model", "openrouter/free");
  microphoneEnabled = storage.getBool("mic", false);
  speakerEnabled = storage.getBool("speaker", false);
  keyboardEnabled = storage.getBool("keyboard", false);
  volume = constrain(storage.getUChar("volume", 50), 0, 100);
  storage.end();
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
