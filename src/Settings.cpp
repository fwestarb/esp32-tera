#include "Settings.h"
#include <Preferences.h>

void Settings::load() {
  Preferences storage;
  if (!storage.begin("tera", true)) return; // First boot has no configuration.
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
