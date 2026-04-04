// ESP32 + 1200 addressable LEDs split across two data pins
// Library needed: FastLED

#include <FastLED.h>

constexpr uint8_t LED_PIN_A = 4;
constexpr uint8_t LED_PIN_B = 16;
constexpr uint16_t LED_COUNT_A = 900;
constexpr uint16_t LED_COUNT_B = 300;
constexpr uint16_t LED_COUNT = LED_COUNT_A + LED_COUNT_B;
constexpr EOrder COLOR_ORDER = GRB;
constexpr uint8_t BRIGHTNESS = 128;
constexpr uint8_t LIT_LEDS = 5;
constexpr uint16_t FRAME_DELAY_MS = 40;

CRGB ledsA[LED_COUNT_A];
CRGB ledsB[LED_COUNT_B];

void setVirtualPixel(uint16_t pixel, const CRGB &color) {
  if (pixel < LED_COUNT_A) {
    ledsA[pixel] = color;
    return;
  }

  ledsB[pixel - LED_COUNT_A] = color;
}

void setup() {
  FastLED.addLeds<WS2812B, LED_PIN_A, COLOR_ORDER>(ledsA, LED_COUNT_A);
  FastLED.addLeds<WS2812B, LED_PIN_B, COLOR_ORDER>(ledsB, LED_COUNT_B);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  static uint16_t head = 0;

  fill_solid(ledsA, LED_COUNT_A, CRGB::Black);
  fill_solid(ledsB, LED_COUNT_B, CRGB::Black);

  for (uint8_t i = 0; i < LIT_LEDS; i++) {
    uint16_t pixel = (head + i) % LED_COUNT;
    setVirtualPixel(pixel, CRGB::White);
  }

  FastLED.show();
  delay(FRAME_DELAY_MS);

  head = (head + 1) % LED_COUNT;
}
