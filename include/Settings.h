#pragma once
#include <Arduino.h>

struct Settings {
  String wifiSsid;
  String wifiPassword;
  String apiKey;
  String textModel = "openrouter/free";
  bool microphoneEnabled = false;
  bool speakerEnabled = false;
  bool keyboardEnabled = false;
  uint8_t volume = 50;
  void load();
};
