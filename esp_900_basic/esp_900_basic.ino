// ESP32 + 900 addressable LEDs on GPIO 4
// Library needed: FastLED

#include <FastLED.h>

constexpr uint8_t LED_PIN = 4;
constexpr uint16_t LED_COUNT = 900;
constexpr EOrder COLOR_ORDER = GRB;
constexpr uint8_t BRIGHTNESS = 128;
constexpr uint8_t LIT_LEDS = 5;
constexpr uint16_t FRAME_DELAY_MS = 40;

CRGB leds[LED_COUNT];

void setup() {
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, LED_COUNT);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  static uint16_t head = 0;

  fill_solid(leds, LED_COUNT, CRGB::Black);

  for (uint8_t i = 0; i < LIT_LEDS; i++) {
    uint16_t pixel = (head + i) % LED_COUNT;
    leds[pixel] = CRGB::White;
  }

  FastLED.show();
  delay(FRAME_DELAY_MS);

  head = (head + 1) % LED_COUNT;
}
