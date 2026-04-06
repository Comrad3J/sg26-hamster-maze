// ESP32 + 1200 addressable LEDs split across two data pins
// Strip A: 900 LEDs on GPIO 21
// Strip B: 300 LEDs on GPIO 4
// Library needed: FastLED

#include <FastLED.h>

constexpr uint8_t LED_PIN_A = 21;
constexpr uint8_t LED_PIN_B = 4;
constexpr uint16_t LED_COUNT_A = 900;
constexpr uint16_t LED_COUNT_B = 300;
constexpr EOrder COLOR_ORDER = RGB;
constexpr uint8_t BRIGHTNESS = 6;
constexpr uint16_t FLASH_DELAY_MS = 80;

CRGB ledsA[LED_COUNT_A];
CRGB ledsB[LED_COUNT_B];

void showColor(const CRGB &color) {
  fill_solid(ledsA, LED_COUNT_A, color);
  fill_solid(ledsB, LED_COUNT_B, color);
  FastLED.show();
}

void setup() {
  FastLED.addLeds<WS2812B, LED_PIN_A, COLOR_ORDER>(ledsA, LED_COUNT_A);
  FastLED.addLeds<WS2812B, LED_PIN_B, COLOR_ORDER>(ledsB, LED_COUNT_B);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);
}

void loop() {
  showColor(CRGB::Red);
  delay(FLASH_DELAY_MS);

  showColor(CRGB::Blue);
  delay(FLASH_DELAY_MS);
}
