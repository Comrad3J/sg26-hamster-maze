#include <FastLED.h>

// Photo-mapping calibration sketch for V2.
// Each strip has its own color and a brighter marker every 10 LEDs.

constexpr uint8_t LED_PIN_1 = 21;   // Strip 1
constexpr uint8_t LED_PIN_2 = 4;    // Strip 2
constexpr uint8_t LED_PIN_3 = 17;   // Strip 3

constexpr uint16_t LED_COUNT_1 = 598;  // LEDs 0..597
constexpr uint16_t LED_COUNT_2 = 290;  // LEDs 0..289
constexpr uint16_t LED_COUNT_3 = 259;  // LEDs 0..258

constexpr EOrder COLOR_ORDER = RGB;
constexpr uint8_t BRIGHTNESS = 40;
constexpr uint8_t GRID_SPACING = 10;

CRGB leds1[LED_COUNT_1];
CRGB leds2[LED_COUNT_2];
CRGB leds3[LED_COUNT_3];

void drawStrip(CRGB *leds, uint16_t count, const CRGB &dotColor) {
  fill_solid(leds, count, CRGB::Black);

  for (uint16_t i = 0; i < count; i += GRID_SPACING) {
    leds[i] = dotColor;
  }
}

void setup() {
  FastLED.addLeds<WS2812B, LED_PIN_1, COLOR_ORDER>(leds1, LED_COUNT_1);
  FastLED.addLeds<WS2812B, LED_PIN_2, COLOR_ORDER>(leds2, LED_COUNT_2);
  FastLED.addLeds<WS2812B, LED_PIN_3, COLOR_ORDER>(leds3, LED_COUNT_3);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);
}

void loop() {
  drawStrip(leds1, LED_COUNT_1, CRGB::Red);
  drawStrip(leds2, LED_COUNT_2, CRGB::Blue);
  drawStrip(leds3, LED_COUNT_3, CRGB::Green);

  FastLED.show();
  delay(20);
}
