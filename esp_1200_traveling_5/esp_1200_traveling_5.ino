#include <FastLED.h>

// V1 hamster movement on the calibrated maze.
// Strip mapping:
//   Strip 1 = GPIO 21, main path with split order B -> D -> C -> D
//   Strip 2 = GPIO 4, short branch from A to C, ending at LED 149
//   Strip 3 = GPIO 17, branch from A to B, ending at LED 258

enum AnimationMode : uint8_t {
  TRAVELING_LIT,
  TRAVELING_DARK
};

enum StripId : uint8_t {
  STRIP_1,
  STRIP_2,
  STRIP_3
};

enum SegmentId : uint8_t {
  SEGMENT_1_1,
  SEGMENT_1_2,
  SEGMENT_1_3,
  SEGMENT_1_4,
  SEGMENT_2,
  SEGMENT_3,
  SEGMENT_COUNT
};

struct Segment {
  StripId strip;
  uint16_t startPixel;
  uint16_t endPixel;
};

struct RouteOption {
  SegmentId segment;
  bool forward;
};

constexpr uint8_t LED_PIN_1 = 21;
constexpr uint8_t LED_PIN_2 = 4;
constexpr uint8_t LED_PIN_3 = 17;
constexpr uint16_t LED_COUNT_1 = 900;
constexpr uint16_t LED_COUNT_2 = 150;   // LEDs 0..149
constexpr uint16_t LED_COUNT_3 = 259;   // LEDs 0..258
constexpr EOrder COLOR_ORDER = RGB;

constexpr AnimationMode MODE = TRAVELING_DARK;
constexpr uint8_t BRIGHTNESS = 35;
constexpr uint8_t HAMSTER_LENGTH = 8;
constexpr uint16_t FRAME_DELAY_MS = 20;
constexpr float SPEED_LEDS_PER_FRAME = 0.7f;

constexpr Segment segments[SEGMENT_COUNT] = {
  {STRIP_1, 0, 300},    // 1_1: A -> B
  {STRIP_1, 301, 440},  // 1_2: B -> D
  {STRIP_1, 441, 597},  // 1_3: D -> C
  {STRIP_1, 598, 737},  // 1_4: C -> D
  {STRIP_2, 0, 149},    // 2: A -> C
  {STRIP_3, 0, 258}     // 3: A -> B
};

CRGB leds1[LED_COUNT_1];
CRGB leds2[LED_COUNT_2];
CRGB leds3[LED_COUNT_3];

uint16_t segmentLength(SegmentId segmentId) {
  const Segment &segment = segments[segmentId];
  return (segment.endPixel - segment.startPixel) + 1;
}

CRGB *ledsForStrip(StripId strip) {
  switch (strip) {
    case STRIP_1:
      return leds1;
    case STRIP_2:
      return leds2;
    case STRIP_3:
      return leds3;
    default:
      return leds1;
  }
}

uint16_t pixelForOffset(SegmentId segmentId, bool forward, uint16_t offset) {
  const Segment &segment = segments[segmentId];
  return forward ? (segment.startPixel + offset) : (segment.endPixel - offset);
}

uint8_t getRouteOptions(SegmentId segmentId, bool arrivedAtStart, RouteOption options[3]) {
  switch (segmentId) {
    case SEGMENT_1_1:
      if (arrivedAtStart) {
        options[0] = {SEGMENT_1_1, true};
        options[1] = {SEGMENT_2, true};
        options[2] = {SEGMENT_3, true};
        return 3;
      }

      options[0] = {SEGMENT_1_1, false};
      options[1] = {SEGMENT_1_2, true};
      options[2] = {SEGMENT_3, false};
      return 3;

    case SEGMENT_1_2:
      if (arrivedAtStart) {
        options[0] = {SEGMENT_1_2, true};
        options[1] = {SEGMENT_1_1, false};
        options[2] = {SEGMENT_3, false};
        return 3;
      }

      options[0] = {SEGMENT_1_2, false};
      options[1] = {SEGMENT_1_3, true};
      options[2] = {SEGMENT_1_4, false};
      return 3;

    case SEGMENT_1_3:
      if (arrivedAtStart) {
        options[0] = {SEGMENT_1_3, true};
        options[1] = {SEGMENT_1_2, false};
        options[2] = {SEGMENT_1_4, false};
        return 3;
      }

      options[0] = {SEGMENT_1_3, false};
      options[1] = {SEGMENT_1_4, true};
      options[2] = {SEGMENT_2, false};
      return 3;

    case SEGMENT_1_4:
      if (arrivedAtStart) {
        options[0] = {SEGMENT_1_4, true};
        options[1] = {SEGMENT_1_3, false};
        options[2] = {SEGMENT_2, false};
        return 3;
      }

      options[0] = {SEGMENT_1_4, false};
      options[1] = {SEGMENT_1_2, false};
      options[2] = {SEGMENT_1_3, true};
      return 3;

    case SEGMENT_2:
      if (arrivedAtStart) {
        options[0] = {SEGMENT_2, true};
        options[1] = {SEGMENT_1_1, true};
        options[2] = {SEGMENT_3, true};
        return 3;
      }

      options[0] = {SEGMENT_2, false};
      options[1] = {SEGMENT_1_3, false};
      options[2] = {SEGMENT_1_4, true};
      return 3;

    case SEGMENT_3:
      if (arrivedAtStart) {
        options[0] = {SEGMENT_3, true};
        options[1] = {SEGMENT_1_1, true};
        options[2] = {SEGMENT_2, true};
        return 3;
      }

      options[0] = {SEGMENT_3, false};
      options[1] = {SEGMENT_1_1, false};
      options[2] = {SEGMENT_1_2, true};
      return 3;

    default:
      options[0] = {SEGMENT_1_1, true};
      return 1;
  }
}

RouteOption chooseNextRoute(SegmentId segmentId, bool arrivedAtStart) {
  RouteOption options[3];
  const uint8_t optionCount = getRouteOptions(segmentId, arrivedAtStart, options);
  return options[random(optionCount)];
}

void drawHamster(SegmentId segmentId, bool forward, int16_t headOffset) {
  const Segment &segment = segments[segmentId];
  CRGB *stripLeds = ledsForStrip(segment.strip);
  const uint16_t length = segmentLength(segmentId);

  for (uint8_t i = 0; i < HAMSTER_LENGTH; i++) {
    const int16_t offset = headOffset - i;
    if (offset < 0 || offset >= static_cast<int16_t>(length)) {
      continue;
    }

    stripLeds[pixelForOffset(segmentId, forward, static_cast<uint16_t>(offset))] =
      (MODE == TRAVELING_DARK) ? CRGB::Black : CRGB::White;
  }
}

void setup() {
  randomSeed(esp_random());

  FastLED.addLeds<WS2812B, LED_PIN_1, COLOR_ORDER>(leds1, LED_COUNT_1);
  FastLED.addLeds<WS2812B, LED_PIN_2, COLOR_ORDER>(leds2, LED_COUNT_2);
  FastLED.addLeds<WS2812B, LED_PIN_3, COLOR_ORDER>(leds3, LED_COUNT_3);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);
}

void loop() {
  static SegmentId currentSegment = SEGMENT_1_1;
  static bool movingForward = true;
  static float position = 0.0f;

  if (MODE == TRAVELING_DARK) {
    fill_solid(leds1, LED_COUNT_1, CRGB::White);
    fill_solid(leds2, LED_COUNT_2, CRGB::White);
    fill_solid(leds3, LED_COUNT_3, CRGB::White);
  } else {
    fill_solid(leds1, LED_COUNT_1, CRGB::Black);
    fill_solid(leds2, LED_COUNT_2, CRGB::Black);
    fill_solid(leds3, LED_COUNT_3, CRGB::Black);
  }

  drawHamster(currentSegment, movingForward, static_cast<int16_t>(position));

  FastLED.show();
  delay(FRAME_DELAY_MS);

  position += SPEED_LEDS_PER_FRAME;

  const uint16_t currentLength = segmentLength(currentSegment);
  if (position < currentLength) {
    return;
  }

  const bool arrivedAtStart = !movingForward;
  const float overflow = position - currentLength;
  const RouteOption nextRoute = chooseNextRoute(currentSegment, arrivedAtStart);

  currentSegment = nextRoute.segment;
  movingForward = nextRoute.forward;
  position = overflow;

  const uint16_t nextLength = segmentLength(currentSegment);
  if (position >= nextLength) {
    position = nextLength - 1;
  }
}
