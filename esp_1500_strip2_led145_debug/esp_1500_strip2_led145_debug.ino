#include <FastLED.h>

// Debug sketch: lights only LED 145 on strip 2.

constexpr uint8_t LED_PIN_2 = 4;
constexpr uint16_t LED_COUNT_2 = 290; // LEDs 0..289
constexpr uint16_t DEBUG_LED = 145;

constexpr EOrder COLOR_ORDER = RGB;
constexpr uint8_t BRIGHTNESS = 170;
constexpr CRGB DEBUG_COLOR = CRGB::Red;

CRGB leds2[LED_COUNT_2];

void setup()
{
  FastLED.addLeds<WS2812B, LED_PIN_2, COLOR_ORDER>(leds2, LED_COUNT_2);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setDither(0);
  FastLED.clear(true);

  if (DEBUG_LED < LED_COUNT_2)
  {
    leds2[DEBUG_LED] = DEBUG_COLOR;
  }

  FastLED.show();
}

void loop()
{
  // Keep the marker stable even if the strip is disturbed by startup noise.
  FastLED.show();
  delay(1000);
}
