// ESP32 + addressable LEDs split across three data pins
// Strip A: 900 LEDs on GPIO 21
// Strip B: 300 LEDs on GPIO 4
// Strip C: 300 LEDs on GPIO 5 (D5)
// Library needed: FastLED

#include <FastLED.h>

enum AnimationMode : uint8_t {
  TRAVELING_LIT,
  TRAVELING_DARK
};

constexpr uint8_t LED_PIN_A = 21; // trak 1
constexpr uint8_t LED_PIN_B = 4;  // trak 2
constexpr uint8_t LED_PIN_C = 17;  // trak 3
constexpr uint16_t LED_COUNT_A = 900;
constexpr uint16_t LED_COUNT_B = 300; 
constexpr uint16_t LED_COUNT_C = 300;
constexpr EOrder COLOR_ORDER = GRB;
constexpr AnimationMode MODE = TRAVELING_DARK;
constexpr uint8_t BRIGHTNESS = 20;
constexpr uint8_t LIT_LEDS = 5;
constexpr uint8_t DARK_LEDS = 7;
constexpr uint16_t FRAME_DELAY_MS = 20;

CRGB ledsA[LED_COUNT_A];
CRGB ledsB[LED_COUNT_B];
CRGB ledsC[LED_COUNT_C];

void setup() {
  FastLED.addLeds<WS2812B, LED_PIN_A, COLOR_ORDER>(ledsA, LED_COUNT_A);
  FastLED.addLeds<WS2812B, LED_PIN_B, COLOR_ORDER>(ledsB, LED_COUNT_B);
  FastLED.addLeds<WS2812B, LED_PIN_C, COLOR_ORDER>(ledsC, LED_COUNT_C);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);
}

void loop() {
  static uint16_t headA = 0;
  static uint16_t headB = 0;
  static uint16_t headC = 0;

  if (MODE == TRAVELING_DARK) {
    fill_solid(ledsA, LED_COUNT_A, CRGB::White);
    fill_solid(ledsB, LED_COUNT_B, CRGB::White);
    fill_solid(ledsC, LED_COUNT_C, CRGB::White);

    for (uint8_t i = 0; i < DARK_LEDS; i++) {
      ledsA[(headA + i) % LED_COUNT_A] = CRGB::Black;
      ledsB[(headB + i) % LED_COUNT_B] = CRGB::Black;
      ledsC[(headC + i) % LED_COUNT_C] = CRGB::Black;
    }
  } else {
    fill_solid(ledsA, LED_COUNT_A, CRGB::Black);
    fill_solid(ledsB, LED_COUNT_B, CRGB::Black);
    fill_solid(ledsC, LED_COUNT_C, CRGB::Black);

    for (uint8_t i = 0; i < LIT_LEDS; i++) {
      ledsA[(headA + i) % LED_COUNT_A] = CRGB::White;
      ledsB[(headB + i) % LED_COUNT_B] = CRGB::White;
      ledsC[(headC + i) % LED_COUNT_C] = CRGB::White;
    }
  }

  FastLED.show();
  delay(FRAME_DELAY_MS);

  headA = (headA + 1) % LED_COUNT_A;
  headB = (headB + 1) % LED_COUNT_B;
  headC = (headC + 1) % LED_COUNT_C;
}
