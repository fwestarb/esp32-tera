#include <Arduino.h>
#include <TFT_eSPI.h>
#include "Board.h"
#include "Settings.h"

TFT_eSPI screen;
Settings settings;
bool showDetails = false;
bool lastMenuPressed = false;
uint32_t lastMenuChange = 0;

constexpr uint16_t background = 0x1082;
constexpr uint16_t panel = 0x2124;
constexpr uint16_t light = 0xD6DA;
constexpr uint16_t green = 0xA6EB;
constexpr uint16_t moss = 0x6C88;

void drawTree(int x, int y) {
  screen.drawLine(x, y, x, y - 24, moss);
  screen.drawLine(x, y - 8, x - 10, y - 19, moss);
  screen.drawLine(x, y - 14, x + 10, y - 26, moss);
  screen.drawLine(x, y - 21, x - 5, y - 28, moss);
}

void render() {
  screen.fillScreen(background);
  screen.fillRoundRect(5, 5, 230, 35, 5, panel);
  drawTree(23, 35);
  screen.setTextColor(green, panel);
  screen.drawString("ESP32 TERA", 43, 14, 2);
  screen.setTextColor(light, background);
  if (showDetails) {
    screen.drawString("TTGO T-Display original", 8, 48, 2);
    screen.drawString("Wi-Fi e IA: proxima etapa", 8, 69, 2);
    screen.drawString("Audio: hardware pendente", 8, 90, 2);
  } else {
    screen.drawString("Base inicial do firmware", 8, 49, 2);
    screen.drawString("Configure antes de conversar", 8, 71, 2);
    screen.drawString("Teclado e voz opcionais", 8, 93, 2);
  }
  screen.setTextColor(moss, background);
  screen.drawString("Botao GPIO0: detalhes", 8, 117, 1);
}

void setup() {
  Serial.begin(115200);
  pinMode(Board::talkButton, INPUT);
  pinMode(Board::menuButton, INPUT_PULLUP);
  settings.load();
  screen.init();
  screen.setRotation(1);
  pinMode(Board::displayBacklight, OUTPUT);
  digitalWrite(Board::displayBacklight, HIGH);
  render();
  Serial.println("ESP32 Tera: firmware base. Rede e audio ainda nao implementados.");
}

void loop() {
  const bool pressed = digitalRead(Board::menuButton) == LOW;
  if (pressed != lastMenuPressed && millis() - lastMenuChange >= 40) {
    lastMenuPressed = pressed;
    lastMenuChange = millis();
    if (pressed) { showDetails = !showDetails; render(); }
  }
  delay(5);
}
