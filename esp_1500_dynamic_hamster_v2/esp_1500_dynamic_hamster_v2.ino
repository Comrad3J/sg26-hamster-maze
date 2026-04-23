#include <FastLED.h>

// V2 dynamic hamster movement on the calibrated maze.
// Uses section length and orientation to vary speed inside each routed strip segment.

enum SectionOrientation : uint8_t
{
  SECTION_HORIZONTAL,
  SECTION_UP,
  SECTION_DOWN
};

enum StripId : uint8_t
{
  STRIP_1,
  STRIP_2,
  STRIP_3
};

enum NodeId : uint8_t
{
  NODE_A,
  NODE_B,
  NODE_C,
  NODE_D
};

enum RouteId : uint8_t
{
  ROUTE_1_A_TO_B,
  ROUTE_1_B_TO_D,
  ROUTE_1_D_TO_C,
  ROUTE_2_A_TO_C,
  ROUTE_2_C_TO_D,
  ROUTE_3_A_TO_B,
  ROUTE_COUNT
};

enum DisplayMode : uint8_t
{
  DISPLAY_NORMAL,
  DISPLAY_INVERTED
};

struct Section
{
  uint16_t length;
  SectionOrientation orientation;
};

struct Route
{
  StripId strip;
  uint16_t startPixel;
  uint16_t endPixel;
  NodeId startNode;
  NodeId endNode;
  const Section *sections;
  uint8_t sectionCount;
};

struct RouteOption
{
  RouteId route;
  bool forward;
};

struct SectionPosition
{
  const Section *section;
  uint16_t startOffset;
  uint16_t endOffset;
  float progress;
};

struct Hamster
{
  RouteId route;
  bool forward;
  float position;
  uint8_t hue;
};

constexpr uint8_t LED_PIN_1 = 21; // Strip 1
constexpr uint8_t LED_PIN_2 = 4;  // Strip 2
constexpr uint8_t LED_PIN_3 = 17; // Strip 3

constexpr uint16_t LED_COUNT_1 = 598; // LEDs 0..597
constexpr uint16_t LED_COUNT_2 = 290; // LEDs 0..289
constexpr uint16_t LED_COUNT_3 = 259; // LEDs 0..258

constexpr EOrder COLOR_ORDER = RGB;
constexpr uint8_t BRIGHTNESS = 60;
constexpr uint8_t HAMSTER_LENGTH = 8;
constexpr uint8_t HAMSTER_COUNT = 2;
constexpr uint16_t FRAME_DELAY_MS = 15;
constexpr uint8_t INTERNAL_BEND_REVERSE_PERCENT = 0;
constexpr float SPEED_MULTIPLIER = 1.75f;
constexpr DisplayMode DISPLAY_MODE = DISPLAY_INVERTED;

const Section strip1Sections[] = {
    {27, SECTION_HORIZONTAL},
    {17, SECTION_UP},
    {3, SECTION_HORIZONTAL},
    {17, SECTION_UP},
    {23, SECTION_HORIZONTAL},
    {8, SECTION_HORIZONTAL},
    {36, SECTION_DOWN},
    {38, SECTION_HORIZONTAL},
    {13, SECTION_UP},
    {5, SECTION_HORIZONTAL},
    {42, SECTION_UP},
    {18, SECTION_HORIZONTAL},
    {35, SECTION_UP},
    {18, SECTION_HORIZONTAL},
    {23, SECTION_DOWN},
    {10, SECTION_HORIZONTAL},
    {50, SECTION_DOWN},
    {30, SECTION_HORIZONTAL},
    {12, SECTION_HORIZONTAL},
    {15, SECTION_UP},
    {23, SECTION_UP},
    {7, SECTION_HORIZONTAL},
    {16, SECTION_UP},
    {10, SECTION_UP},
    {22, SECTION_HORIZONTAL},
    {5, SECTION_HORIZONTAL},
    {30, SECTION_DOWN},
    {44, SECTION_UP}};

const Section strip2Sections[] = {
    {60, SECTION_UP},
    {5, SECTION_HORIZONTAL},
    {20, SECTION_UP},
    {18, SECTION_HORIZONTAL},
    {25, SECTION_UP},
    {21, SECTION_HORIZONTAL},
    {12, SECTION_HORIZONTAL},
    {100, SECTION_DOWN},
    {28, SECTION_UP}};

const Section strip3Sections[] = {
    {15, SECTION_UP},
    {33, SECTION_HORIZONTAL},
    {48, SECTION_UP},
    {13, SECTION_HORIZONTAL},
    {36, SECTION_UP},
    {20, SECTION_HORIZONTAL},
    {25, SECTION_HORIZONTAL},
    {5, SECTION_HORIZONTAL},
    {5, SECTION_DOWN},
    {12, SECTION_HORIZONTAL},
    {30, SECTION_HORIZONTAL},
    {5, SECTION_DOWN},
    {11, SECTION_HORIZONTAL}};

const Route routes[ROUTE_COUNT] = {
    {STRIP_1, 0, 300, NODE_A, NODE_B, strip1Sections, 14},
    {STRIP_1, 300, 440, NODE_B, NODE_D, strip1Sections + 14, 6},
    {STRIP_1, 440, 597, NODE_D, NODE_C, strip1Sections + 20, 8},
    {STRIP_2, 0, 149, NODE_A, NODE_C, strip2Sections, 6},
    {STRIP_2, 149, 289, NODE_C, NODE_D, strip2Sections + 6, 3},
    {STRIP_3, 0, 258, NODE_A, NODE_B, strip3Sections, 13}};

CRGB leds1[LED_COUNT_1];
CRGB leds2[LED_COUNT_2];
CRGB leds3[LED_COUNT_3];

Hamster hamsters[HAMSTER_COUNT] = {
    {ROUTE_1_A_TO_B, true, 0.0f, 34},
    {ROUTE_2_A_TO_C, true, 0.0f, 145}};

CRGB *ledsForStrip(StripId strip)
{
  switch (strip)
  {
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

uint16_t ledCountForStrip(StripId strip)
{
  switch (strip)
  {
  case STRIP_1:
    return LED_COUNT_1;
  case STRIP_2:
    return LED_COUNT_2;
  case STRIP_3:
    return LED_COUNT_3;
  default:
    return LED_COUNT_1;
  }
}

uint16_t routeLength(RouteId routeId)
{
  const Route &route = routes[routeId];
  uint16_t total = 0;

  for (uint8_t i = 0; i < route.sectionCount; i++)
  {
    total += route.sections[i].length;
  }

  return total;
}

uint16_t pixelForRoutePosition(RouteId routeId, float position)
{
  const Route &route = routes[routeId];
  const uint16_t length = routeLength(routeId);
  int16_t offset = static_cast<int16_t>(position + 0.5f);

  if (offset < 0)
  {
    offset = 0;
  }
  if (offset > static_cast<int16_t>(length))
  {
    offset = length;
  }

  return route.startPixel + offset;
}

SectionPosition findSectionPosition(RouteId routeId, float routePosition, bool forward)
{
  const Route &route = routes[routeId];

  if (forward)
  {
    uint16_t start = 0;
    for (uint8_t i = 0; i < route.sectionCount; i++)
    {
      const uint16_t end = start + route.sections[i].length;
      if (routePosition < end || i == route.sectionCount - 1)
      {
        const float local = routePosition - start;
        const float progress = constrain(local / route.sections[i].length, 0.0f, 1.0f);
        return {route.sections + i, start, end, progress};
      }
      start = end;
    }
  }

  uint16_t end = routeLength(routeId);
  for (int8_t i = route.sectionCount - 1; i >= 0; i--)
  {
    const uint16_t start = end - route.sections[i].length;
    if (routePosition > start || i == 0)
    {
      const float local = end - routePosition;
      const float progress = constrain(local / route.sections[i].length, 0.0f, 1.0f);
      return {route.sections + i, start, end, progress};
    }
    end = start;
  }

  return {route.sections, 0, route.sections[0].length, 0.0f};
}

float smooth01(float value)
{
  value = constrain(value, 0.0f, 1.0f);
  return value * value * (3.0f - 2.0f * value);
}

float maxSpeedForLength(uint16_t sectionLength)
{
  return constrain(0.32f + (sectionLength * 0.018f), 0.38f, 1.85f);
}

bool isMovingDown(const Section &section, bool forward)
{
  if (section.orientation == SECTION_HORIZONTAL)
  {
    return false;
  }

  return (section.orientation == SECTION_DOWN && forward) ||
         (section.orientation == SECTION_UP && !forward);
}

bool isMovingUp(const Section &section, bool forward)
{
  if (section.orientation == SECTION_HORIZONTAL)
  {
    return false;
  }

  return !isMovingDown(section, forward);
}

float speedForCurrentPosition(const Hamster &state)
{
  const SectionPosition sectionPosition =
      findSectionPosition(state.route, state.position, state.forward);
  const Section &section = *sectionPosition.section;
  const float progress = sectionPosition.progress;
  const float maxSpeed = maxSpeedForLength(section.length);
  const float longSectionPenalty = 1.0f - (0.55f * smooth01((section.length - 18.0f) / 82.0f));
  const float upwardBase = maxSpeed * (0.28f + (0.10f * smooth01(progress)));
  const float upwardEquivalent = constrain(upwardBase * longSectionPenalty, 0.22f, maxSpeed * 0.38f);

  if (isMovingUp(section, state.forward))
  {
    return SPEED_MULTIPLIER * upwardEquivalent;
  }

  if (isMovingDown(section, state.forward))
  {
    const float gravity = 0.42f + (0.34f * smooth01(progress));
    const float suddenBrake = 1.0f - (0.82f * smooth01((progress - 0.86f) / 0.14f));
    return SPEED_MULTIPLIER * constrain(maxSpeed * gravity * suddenBrake, 0.08f, maxSpeed * 0.72f);
  }

  const float bendCurve = sin(progress * PI);
  const float horizontalSpeed = constrain(maxSpeed * (0.24f + (0.76f * bendCurve)), 0.20f, maxSpeed) * 0.50f;
  return SPEED_MULTIPLIER * max(horizontalSpeed, upwardEquivalent);
}

uint8_t routeOptionsForNode(NodeId node, RouteOption options[3])
{
  uint8_t count = 0;

  for (uint8_t i = 0; i < ROUTE_COUNT && count < 3; i++)
  {
    const Route &route = routes[i];
    if (route.startNode == node)
    {
      options[count++] = {static_cast<RouteId>(i), true};
    }
    else if (route.endNode == node)
    {
      options[count++] = {static_cast<RouteId>(i), false};
    }
  }

  return count;
}

void chooseRouteFromNode(Hamster &state, NodeId node)
{
  RouteOption options[3];
  const uint8_t optionCount = routeOptionsForNode(node, options);

  if (optionCount == 0)
  {
    state.forward = !state.forward;
    return;
  }

  RouteOption selectableOptions[3];
  uint8_t selectableCount = 0;
  const bool reversingCurrentRoute = !state.forward;

  for (uint8_t i = 0; i < optionCount; i++)
  {
    const bool isImmediateReverse =
        options[i].route == state.route &&
        options[i].forward == reversingCurrentRoute;

    if (!isImmediateReverse || optionCount == 1)
    {
      selectableOptions[selectableCount++] = options[i];
    }
  }

  if (selectableCount == 0)
  {
    for (uint8_t i = 0; i < optionCount; i++)
    {
      selectableOptions[selectableCount++] = options[i];
    }
  }

  const RouteOption next = selectableOptions[random(selectableCount)];
  state.route = next.route;
  state.forward = next.forward;
  state.position = next.forward ? 0.0f : routeLength(next.route);
}

void handleBoundary(Hamster &state, uint16_t boundary)
{
  const Route &route = routes[state.route];
  const uint16_t length = routeLength(state.route);
  state.position = boundary;

  if (boundary == 0)
  {
    chooseRouteFromNode(state, route.startNode);
    return;
  }

  if (boundary == length)
  {
    chooseRouteFromNode(state, route.endNode);
    return;
  }

  if (random(100) < INTERNAL_BEND_REVERSE_PERCENT)
  {
    state.forward = !state.forward;
  }
}

void updateHamster(Hamster &state)
{
  const SectionPosition sectionPosition =
      findSectionPosition(state.route, state.position, state.forward);
  const float speed = speedForCurrentPosition(state);
  const uint16_t boundary = state.forward ? sectionPosition.endOffset : sectionPosition.startOffset;
  const float nextPosition = state.position + (state.forward ? speed : -speed);

  if ((state.forward && nextPosition >= boundary) ||
      (!state.forward && nextPosition <= boundary))
  {
    handleBoundary(state, boundary);
    return;
  }

  state.position = nextPosition;
}

void drawBackground()
{
  const CRGB background = (DISPLAY_MODE == DISPLAY_INVERTED) ? CRGB::White : CRGB::Black;

  fill_solid(leds1, LED_COUNT_1, background);
  fill_solid(leds2, LED_COUNT_2, background);
  fill_solid(leds3, LED_COUNT_3, background);
}

CRGB invertedHamsterColor(uint8_t trailIndex)
{
  return CRGB::Black;
}

void drawHamster(const Hamster &state)
{
  CRGB hamsterColor = CRGB::Black;

  const Route &route = routes[state.route];
  CRGB *leds = ledsForStrip(route.strip);
  const uint16_t ledCount = ledCountForStrip(route.strip);
  const int8_t trailDirection = state.forward ? -1 : 1;

  for (uint8_t i = 0; i < HAMSTER_LENGTH; i++)
  {
    const float trailPosition = state.position + (trailDirection * i);
    if (trailPosition < 0.0f || trailPosition > routeLength(state.route))
    {
      continue;
    }

    const uint16_t pixel = pixelForRoutePosition(state.route, trailPosition);
    if (pixel >= ledCount)
    {
      continue;
    }

    if (DISPLAY_MODE == DISPLAY_NORMAL)
    {
      const uint8_t brightness = 255 - (i * (170 / HAMSTER_LENGTH));
      hamsterColor = CHSV(state.hue, 210, brightness);
    }
    else
    {
      hamsterColor = invertedHamsterColor(i);
    }

    leds[pixel] = hamsterColor;
  }
}

void setup()
{
  randomSeed(esp_random());

  FastLED.addLeds<WS2812B, LED_PIN_1, COLOR_ORDER>(leds1, LED_COUNT_1);
  FastLED.addLeds<WS2812B, LED_PIN_2, COLOR_ORDER>(leds2, LED_COUNT_2);
  FastLED.addLeds<WS2812B, LED_PIN_3, COLOR_ORDER>(leds3, LED_COUNT_3);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setDither(0);
  FastLED.clear(true);
}

void loop()
{
  drawBackground();
  for (uint8_t i = 0; i < HAMSTER_COUNT; i++)
  {
    drawHamster(hamsters[i]);
  }

  FastLED.show();
  delay(FRAME_DELAY_MS);

  for (uint8_t i = 0; i < HAMSTER_COUNT; i++)
  {
    updateHamster(hamsters[i]);
  }
}
