#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

struct Settings {
  String wifiSsid;
  String wifiPassword;
  String apiKey;
  String provider = "groq";
  void load();
  bool save();
  bool valid(String& error) const;
  static bool parse(JsonObjectConst doc, const Settings& current, bool keepBlank, Settings& next, String& error);
};
