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
constexpr float DIRECTION_CHANGE_CHANCE = 0.001f;

// Motion tuning.
constexpr float BASE_SPEED = 1.2f;
constexpr float TURN_SPEED = 0.28f;
constexpr float SPEED_RESPONSE = 0.18f;
constexpr uint16_t DEFAULT_TURN_RADIUS = 20;

struct TurnPoint {
  uint16_t position;
  uint16_t radius;
  float minSpeed;
};

// Add or change right-angle positions here.
TurnPoint turns[] = {
  {120, DEFAULT_TURN_RADIUS, TURN_SPEED},
  {260, DEFAULT_TURN_RADIUS, TURN_SPEED},
  {430, DEFAULT_TURN_RADIUS, TURN_SPEED},
  {610, DEFAULT_TURN_RADIUS, TURN_SPEED},
  {790, DEFAULT_TURN_RADIUS, TURN_SPEED},
  {980, DEFAULT_TURN_RADIUS, TURN_SPEED},
  {1120, DEFAULT_TURN_RADIUS, TURN_SPEED},
};

constexpr uint8_t TURN_COUNT = sizeof(turns) / sizeof(turns[0]);

CRGB ledsA[LED_COUNT_A];
CRGB ledsB[LED_COUNT_B];

void setVirtualPixel(uint16_t pixel, const CRGB &color) {
  if (pixel < LED_COUNT_A) {
    ledsA[pixel] = color;
    return;
  }

  ledsB[pixel - LED_COUNT_A] = color;
}

float wrappedDistance(float a, float b) {
  float distance = fabsf(a - b);
  if (distance > (LED_COUNT / 2.0f)) {
    distance = LED_COUNT - distance;
  }
  return distance;
}

float targetSpeedForPosition(float position) {
  float targetSpeed = BASE_SPEED;

  for (uint8_t i = 0; i < TURN_COUNT; i++) {
    float distance = wrappedDistance(position, turns[i].position);
    if (distance > turns[i].radius) {
      continue;
    }

    float blend = distance / turns[i].radius;
    float localSpeed = turns[i].minSpeed + (BASE_SPEED - turns[i].minSpeed) * blend;
    if (localSpeed < targetSpeed) {
      targetSpeed = localSpeed;
    }
  }

  return targetSpeed;
}

void setup() {
  randomSeed(esp_random());
  FastLED.addLeds<WS2812B, LED_PIN_A, COLOR_ORDER>(ledsA, LED_COUNT_A);
  FastLED.addLeds<WS2812B, LED_PIN_B, COLOR_ORDER>(ledsB, LED_COUNT_B);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  static float head = 0.0f;
  static float currentSpeed = BASE_SPEED;
  static int8_t direction = 1;

  fill_solid(ledsA, LED_COUNT_A, CRGB::Black);
  fill_solid(ledsB, LED_COUNT_B, CRGB::Black);

  float targetSpeed = targetSpeedForPosition(head);
  currentSpeed += (targetSpeed - currentSpeed) * SPEED_RESPONSE;

  if (random(10000) < static_cast<long>(DIRECTION_CHANGE_CHANCE * 10000.0f)) {
    direction *= -1;
  }

  for (uint8_t i = 0; i < LIT_LEDS; i++) {
    int32_t pixel = static_cast<int32_t>(head) + (i * direction);
    while (pixel < 0) {
      pixel += LED_COUNT;
    }
    while (pixel >= LED_COUNT) {
      pixel -= LED_COUNT;
    }
    setVirtualPixel(pixel, CRGB::White);
  }

  FastLED.show();
  delay(FRAME_DELAY_MS);

  head += currentSpeed * direction;
  while (head < 0.0f) {
    head += LED_COUNT;
  }
  while (head >= LED_COUNT) {
    head -= LED_COUNT;
  }
}
