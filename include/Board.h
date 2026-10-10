#pragma once

namespace Board {
constexpr int displayBacklight = 4;
constexpr int talkButton = 35; // External pull-up on the original board.
constexpr int menuButton = 0; // Boot strap: do not hold during reset.
constexpr int keyboardSda = 21;
constexpr int keyboardScl = 22;
constexpr unsigned char keyboardAddress = 0x5F;
constexpr int microphone = 32; // Analog amplifier OUT, ADC1 (works with WiFi).
}
