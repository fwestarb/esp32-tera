#include <Arduino.h>
#include <TFT_eSPI.h>
#include <math.h>
#include "Board.h"

// Analog preamplifier output only. GPIO32 must stay within 0..3.3 V.
constexpr int micPin = 32; // ADC1: compatible with future Wi-Fi operation.
constexpr unsigned samplesPerWindow = 800;
constexpr uint32_t samplePeriodUs = 125; // Nominal 8 kHz; a level test, not recording.
TFT_eSPI display;
float referenceRms = 0;
bool calibrated = false;
bool lastPressed = false;
uint32_t lastButton = 0;
unsigned calibrationWindows = 0;
float calibrationSum = 0;

void setup() {
  Serial.begin(115200);
  pinMode(micPin, INPUT);
  pinMode(Board::menuButton, INPUT_PULLUP);
  analogReadResolution(12);
  analogSetPinAttenuation(micPin, ADC_11db);
  display.init(); display.setRotation(1);
  pinMode(Board::displayBacklight, OUTPUT);
  digitalWrite(Board::displayBacklight, HIGH);
  display.fillScreen(0x1082);
  display.setTextColor(0xA6EB, 0x1082);
  display.drawString("TERA - TESTE MICROFONE", 5, 5, 2);
  Serial.println("MIC_TEST GPIO32; fique em silencio por 3 segundos; GPIO0 recalibra.");
  Serial.println("media,pico_a_pico,rms,ruido_rms,clipping");
}

void loop() {
  unsigned minimum = 4095, maximum = 0, clipped = 0;
  uint64_t sum = 0, sumSquares = 0;
  uint32_t next = micros();
  for (unsigned i = 0; i < samplesPerWindow; ++i) {
    while (static_cast<int32_t>(micros() - next) < 0) {}
    next += samplePeriodUs;
    unsigned value = analogRead(micPin);
    minimum = min(minimum, value); maximum = max(maximum, value);
    sum += value; sumSquares += static_cast<uint64_t>(value) * value;
    if (value <= 10 || value >= 4085) ++clipped;
  }
  float mean = static_cast<float>(sum) / samplesPerWindow;
  float variance = static_cast<float>(sumSquares) / samplesPerWindow - mean * mean;
  float rms = sqrtf(max(0.0f, variance)); // Remove DC bias before measuring activity.
  if (!calibrated) {
    calibrationSum += rms;
    if (++calibrationWindows >= 30) {
      referenceRms = calibrationSum / calibrationWindows;
      calibrated = true;
    }
  }
  bool pressed = digitalRead(Board::menuButton) == LOW;
  if (pressed != lastPressed && millis() - lastButton >= 40) {
    lastButton = millis(); lastPressed = pressed;
    if (pressed) { calibrated = false; calibrationSum = 0; calibrationWindows = 0; }
  }
  float threshold = max(12.0f, referenceRms * 2.5f);
  const char* state = !calibrated ? "Silencio: calibrando..." : clipped > 8 ? "Sinal no limite do ADC" : rms > threshold ? "Variacao acima do ruido" : "Sem variacao forte";
  display.fillRect(0, 30, 240, 105, 0x1082);
  display.setTextColor(TFT_LIGHTGREY, 0x1082);
  display.drawString(String("Media: ") + String(mean, 0) + "  P-P: " + String(maximum - minimum), 5, 32, 2);
  display.drawString(String("RMS: ") + String(rms, 1) + "  Base: " + String(referenceRms, 1), 5, 54, 2);
  display.setTextColor(0xA6EB, 0x1082);
  display.drawString(state, 5, 78, 2);
  display.drawRect(5, 101, 230, 13, 0x6C88);
  int width = constrain(static_cast<int>(rms * 225 / max(50.0f, threshold * 3)), 0, 225);
  display.fillRect(7, 103, width, 9, 0xA6EB);
  display.drawString("GPIO0: recalibrar | GPIO32: ADC", 5, 121, 1);
  Serial.printf("%.1f,%u,%.2f,%.2f,%u\n", mean, maximum - minimum, rms, referenceRms, clipped);
  // Electrical activity alone does not prove that the microphone captured speech.
  yield();
}
