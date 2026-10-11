#pragma once
#include <Arduino.h>
// Returns an allowlisted target, "clarify" for unsupported/ambiguous requests,
// or empty for normal conversation. Never extracts executable command text.
String classifyAction(String text);
