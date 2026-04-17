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
  ROUTE_1_C_TO_D,
  ROUTE_2_A_TO_C,
  ROUTE_3_A_TO_B,
  ROUTE_COUNT
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
};

constexpr uint8_t LED_PIN_1 = 21; // Strip 1
constexpr uint8_t LED_PIN_2 = 4;  // Strip 2
constexpr uint8_t LED_PIN_3 = 17; // Strip 3

constexpr uint16_t LED_COUNT_1 = 738; // LEDs 0..737
constexpr uint16_t LED_COUNT_2 = 150; // LEDs 0..149
constexpr uint16_t LED_COUNT_3 = 259; // LEDs 0..258

constexpr EOrder COLOR_ORDER = RGB;
constexpr uint8_t BRIGHTNESS = 95;
constexpr uint8_t HAMSTER_LENGTH = 8;
constexpr uint16_t FRAME_DELAY_MS = 20;
constexpr uint8_t INTERNAL_BEND_REVERSE_CHANCE = 3;

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
    {44, SECTION_UP},
    {12, SECTION_HORIZONTAL},
    {100, SECTION_DOWN},
    {28, SECTION_UP}};

const Section strip2Sections[] = {
    {60, SECTION_UP},
    {5, SECTION_HORIZONTAL},
    {20, SECTION_UP},
    {18, SECTION_HORIZONTAL},
    {25, SECTION_UP},
    {21, SECTION_HORIZONTAL}};

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
    {STRIP_1, 597, 737, NODE_C, NODE_D, strip1Sections + 28, 3},
    {STRIP_2, 0, 149, NODE_A, NODE_C, strip2Sections, 6},
    {STRIP_3, 0, 258, NODE_A, NODE_B, strip3Sections, 13}};

CRGB leds1[LED_COUNT_1];
CRGB leds2[LED_COUNT_2];
CRGB leds3[LED_COUNT_3];

Hamster hamster = {ROUTE_1_A_TO_B, true, 0.0f};

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

  if (isMovingUp(section, state.forward))
  {
    const float jitter = static_cast<float>(random(-8, 9)) / 100.0f;
    const float bendBrake = 0.78f + (0.22f * sin(progress * PI));
    return constrain((maxSpeed * 0.46f * bendBrake) + jitter, 0.18f, maxSpeed * 0.72f);
  }

  if (isMovingDown(section, state.forward))
  {
    const float gravity = 0.28f + (0.92f * smooth01(progress));
    const float endBrake = 1.0f - (0.42f * smooth01((progress - 0.78f) / 0.22f));
    return constrain(maxSpeed * gravity * endBrake, 0.22f, maxSpeed);
  }

  const float bendCurve = sin(progress * PI);
  return constrain(maxSpeed * (0.24f + (0.76f * bendCurve)), 0.20f, maxSpeed);
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

void chooseRouteFromNode(NodeId node)
{
  RouteOption options[3];
  const uint8_t optionCount = routeOptionsForNode(node, options);

  if (optionCount == 0)
  {
    hamster.forward = !hamster.forward;
    return;
  }

  const RouteOption next = options[random(optionCount)];
  hamster.route = next.route;
  hamster.forward = next.forward;
  hamster.position = next.forward ? 0.0f : routeLength(next.route);
}

void handleBoundary(uint16_t boundary)
{
  const Route &route = routes[hamster.route];
  const uint16_t length = routeLength(hamster.route);
  hamster.position = boundary;

  if (boundary == 0)
  {
    chooseRouteFromNode(route.startNode);
    return;
  }

  if (boundary == length)
  {
    chooseRouteFromNode(route.endNode);
    return;
  }

  if (random(INTERNAL_BEND_REVERSE_CHANCE) == 0)
  {
    hamster.forward = !hamster.forward;
  }
}

void updateHamster()
{
  const SectionPosition sectionPosition =
      findSectionPosition(hamster.route, hamster.position, hamster.forward);
  const float speed = speedForCurrentPosition(hamster);
  const uint16_t boundary = hamster.forward ? sectionPosition.endOffset : sectionPosition.startOffset;
  const float nextPosition = hamster.position + (hamster.forward ? speed : -speed);

  if ((hamster.forward && nextPosition >= boundary) ||
      (!hamster.forward && nextPosition <= boundary))
  {
    handleBoundary(boundary);
    return;
  }

  hamster.position = nextPosition;
}

void drawHamster()
{
  fill_solid(leds1, LED_COUNT_1, CRGB::Black);
  fill_solid(leds2, LED_COUNT_2, CRGB::Black);
  fill_solid(leds3, LED_COUNT_3, CRGB::Black);

  const Route &route = routes[hamster.route];
  CRGB *leds = ledsForStrip(route.strip);
  const uint16_t ledCount = ledCountForStrip(route.strip);
  const int8_t trailDirection = hamster.forward ? -1 : 1;

  for (uint8_t i = 0; i < HAMSTER_LENGTH; i++)
  {
    const float trailPosition = hamster.position + (trailDirection * i);
    if (trailPosition < 0.0f || trailPosition > routeLength(hamster.route))
    {
      continue;
    }

    const uint16_t pixel = pixelForRoutePosition(hamster.route, trailPosition);
    if (pixel >= ledCount)
    {
      continue;
    }

    const uint8_t brightness = 255 - (i * (170 / HAMSTER_LENGTH));
    leds[pixel] = CHSV(34, 210, brightness);
  }
}

void setup()
{
  randomSeed(esp_random());

  FastLED.addLeds<WS2812B, LED_PIN_1, COLOR_ORDER>(leds1, LED_COUNT_1);
  FastLED.addLeds<WS2812B, LED_PIN_2, COLOR_ORDER>(leds2, LED_COUNT_2);
  FastLED.addLeds<WS2812B, LED_PIN_3, COLOR_ORDER>(leds3, LED_COUNT_3);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);
}

void loop()
{
  drawHamster();
  FastLED.show();
  delay(FRAME_DELAY_MS);
  updateHamster();
}
