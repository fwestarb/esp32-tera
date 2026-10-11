#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

struct Settings {
  String wifiSsid;
  String wifiPassword;
  String apiKey;
  String provider = "groq";
  String model = "openai/gpt-oss-120b";
  static String defaultModel(const String& provider);
  static bool allowedModel(const String& provider, const String& model);
  void load();
  bool save();
  bool valid(String& error) const;
  static bool parse(JsonObjectConst doc, const Settings& current, bool keepBlank, Settings& next, String& error);
};
