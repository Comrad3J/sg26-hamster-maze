// ESP32 + addressable LEDs
// Turns all LEDs on at very low brightness for strip testing.
// Library needed: FastLED

#include <FastLED.h>

constexpr uint8_t LED_PIN = 4;
constexpr uint16_t LED_COUNT = 600;
constexpr EOrder COLOR_ORDER = GRB;
constexpr uint8_t BRIGHTNESS = 0;

CRGB leds[LED_COUNT];

void setup() {
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, LED_COUNT);
  FastLED.setBrightness(BRIGHTNESS);
  fill_solid(leds, LED_COUNT, CRGB::White);
  FastLED.show();
}

void loop() {
}
