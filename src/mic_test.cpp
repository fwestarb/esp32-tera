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
constexpr unsigned audioSamples = 40000; // Five seconds at nominal 8 kHz.
uint16_t audioBuffer[audioSamples];

uint32_t audioCrc(const uint8_t* bytes, size_t length) {
  uint32_t crc = 0xffffffff;
  while (length--) {
    crc ^= *bytes++;
    for (unsigned bit=0; bit<8; ++bit) crc = (crc >> 1) ^ (0xedb88320U & (0U - (crc & 1U)));
  }
  return crc ^ 0xffffffff;
}

void recordAudio() {
  for (int seconds=3; seconds>0; --seconds) {
    display.fillRect(0,30,240,105,0x1082);
    display.drawString(String("Fale em ")+seconds+" segundos",5,48,2);
    delay(1000);
  }
  display.fillRect(0,30,240,105,0x1082);
  display.drawString("GRAVANDO - fale agora",5,48,2);
  uint32_t start=micros(), next=start, late=0;
  for (unsigned i=0; i<audioSamples; ++i) {
    while (static_cast<int32_t>(micros()-next)<0) {}
    if (static_cast<int32_t>(micros()-next)>30) ++late;
    audioBuffer[i]=analogRead(micPin);
    next+=samplePeriodUs;
    // Yield between chunks; record any timing gaps instead of hiding them.
    if ((i+1)%8000==0 && i+1<audioSamples) delay(1);
  }
  uint32_t elapsed=micros()-start;
  display.fillRect(0,30,240,105,0x1082);
  display.drawString("Enviando audio pela USB",5,48,2);
  const uint8_t* bytes=reinterpret_cast<const uint8_t*>(audioBuffer);
  uint32_t crc=audioCrc(bytes,sizeof(audioBuffer));
  Serial.printf("AUDIO1 %u %u %u %u\n",audioSamples,elapsed,late,crc);
  Serial.write(bytes,sizeof(audioBuffer));Serial.flush();Serial.println("\nAUDIO_END");
}

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
  Serial.println("MIC_TEST GPIO32; GPIO0 ou comando r gravam 5 segundos.");
  Serial.println("media,pico_a_pico,rms,ruido_rms,clipping");
}

void loop() {
  while (Serial.available()) {
    char command=Serial.read();
    if (command=='r') recordAudio();
  }
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
    if (pressed) recordAudio();
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
  display.drawString("GPIO0: gravar 5s | GPIO32: ADC", 5, 121, 1);
  Serial.printf("%.1f,%u,%.2f,%.2f,%u\n", mean, maximum - minimum, rms, referenceRms, clipped);
  // Electrical activity alone does not prove that the microphone captured speech.
  yield();
}
