#include <FastLED.h>

// Bright section calibration sketch for V2 dynamic hamster movement.
// The section map stores contiguous tube lengths plus their physical orientation.

enum SectionOrientation : uint8_t
{
  SECTION_HORIZONTAL,
  SECTION_UP,
  SECTION_DOWN
};

struct StripSection
{
  uint16_t length;
  SectionOrientation orientation;
};

struct StripSectionMap
{
  CRGB *leds;
  uint16_t ledCount;
  const StripSection *sections;
  uint8_t sectionCount;
  bool enabled;
  uint16_t hideBeforePixel;
};

constexpr uint8_t LED_PIN_1 = 21; // Strip 1
constexpr uint8_t LED_PIN_2 = 4;  // Strip 2
constexpr uint8_t LED_PIN_3 = 17; // Strip 3

constexpr uint16_t LED_COUNT_1 = 738; // LEDs 0..737
constexpr uint16_t LED_COUNT_2 = 150; // LEDs 0..149
constexpr uint16_t LED_COUNT_3 = 259; // LEDs 0..258

constexpr EOrder COLOR_ORDER = RGB;
constexpr uint8_t BRIGHTNESS = 170; // 255 full brightness
constexpr uint8_t GRID_SPACING = 10;
constexpr uint8_t BOUNDARY_WIDTH = 2;
constexpr uint8_t NEXT_SECTION_MARKER_WIDTH = 5;
constexpr bool SHOW_STRIP_1_SECTIONS = true;
constexpr bool SHOW_STRIP_2_SECTIONS = false;
constexpr bool SHOW_STRIP_3_SECTIONS = false;
constexpr uint16_t STRIP_1_HIDE_BEFORE_PIXEL = 597; // Keeps LEDs 0..596 off while mapping after split C.
constexpr uint16_t STRIP_2_HIDE_BEFORE_PIXEL = 0;
constexpr uint16_t STRIP_3_HIDE_BEFORE_PIXEL = 0;

const StripSection strip2Sections[] = {
    {60, SECTION_UP},
    {5, SECTION_HORIZONTAL},
    {20, SECTION_UP},
    {18, SECTION_HORIZONTAL},
    {25, SECTION_UP},
    {21, SECTION_HORIZONTAL}};

const StripSection strip3Sections[] = {
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
    {11, SECTION_HORIZONTAL},
};

const StripSection strip1Sections[] = {
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
    // COMES IN SPLIT B (LENGTH SHOULD BE 300)
    {23, SECTION_DOWN},
    {10, SECTION_HORIZONTAL},
    {50, SECTION_DOWN},
    {30, SECTION_HORIZONTAL},
    {12, SECTION_HORIZONTAL},
    {15, SECTION_UP},
    // COMES IN SPLIT D (LENGTH SHOULD BE 440))
    {23, SECTION_UP},
    {7, SECTION_HORIZONTAL},
    {16, SECTION_UP},
    {10, SECTION_UP},
    {22, SECTION_HORIZONTAL},
    {5, SECTION_HORIZONTAL},
    {30, SECTION_DOWN},
    {44, SECTION_UP},
    // PASSES THORUGH SPLIT C (SHOULD BE 597)
    {12, SECTION_HORIZONTAL},
    {100, SECTION_DOWN},
    {28, SECTION_UP}
    // LOOPS BACK TO D (SHOULD BE 737 here)
};

constexpr uint8_t STRIP_1_SECTION_COUNT =
    sizeof(strip1Sections) / sizeof(strip1Sections[0]);

constexpr uint8_t STRIP_2_SECTION_COUNT =
    sizeof(strip2Sections) / sizeof(strip2Sections[0]);

constexpr uint8_t STRIP_3_SECTION_COUNT =
    sizeof(strip3Sections) / sizeof(strip3Sections[0]);

CRGB leds1[LED_COUNT_1];
CRGB leds2[LED_COUNT_2];
CRGB leds3[LED_COUNT_3];

const StripSectionMap stripSectionMaps[] = {
    {leds1, LED_COUNT_1, strip1Sections, STRIP_1_SECTION_COUNT, SHOW_STRIP_1_SECTIONS, STRIP_1_HIDE_BEFORE_PIXEL},
    {leds2, LED_COUNT_2, strip2Sections, STRIP_2_SECTION_COUNT, SHOW_STRIP_2_SECTIONS, STRIP_2_HIDE_BEFORE_PIXEL},
    {leds3, LED_COUNT_3, strip3Sections, STRIP_3_SECTION_COUNT, SHOW_STRIP_3_SECTIONS, STRIP_3_HIDE_BEFORE_PIXEL}};

constexpr uint8_t STRIP_COUNT =
    sizeof(stripSectionMaps) / sizeof(stripSectionMaps[0]);

CRGB sectionColor(uint8_t sectionIndex, SectionOrientation orientation)
{
  switch (sectionIndex % 6)
  {
  case 0:
    return CRGB(0, 255, 0); // green
  case 1:
    return CRGB(255, 90, 0); // orange
  case 2:
    return CRGB(0, 210, 255); // cyan
  case 3:
    return CRGB(255, 0, 180); // magenta
  case 4:
    return CRGB(255, 230, 0); // yellow
  default:
    return (orientation == SECTION_DOWN) ? CRGB(80, 80, 255) : CRGB(255, 0, 0);
  }
}

uint16_t totalMappedLength(const StripSection *sections, uint8_t sectionCount)
{
  uint16_t total = 0;

  for (uint8_t i = 0; i < sectionCount; i++)
  {
    total += sections[i].length;
  }

  return total;
}

void drawMarker(CRGB *leds, uint16_t count, uint16_t pixel, uint8_t width, const CRGB &color)
{
  if (pixel >= count)
  {
    return;
  }

  uint16_t endPixel = pixel + width - 1;
  if (endPixel >= count)
  {
    endPixel = count - 1;
  }

  for (uint16_t i = pixel; i <= endPixel; i++)
  {
    leds[i] = color;
  }
}

void drawMarkerAfter(CRGB *leds, uint16_t count, uint16_t pixel, uint8_t width, const CRGB &color, uint16_t firstVisiblePixel)
{
  if (pixel < firstVisiblePixel)
  {
    return;
  }

  drawMarker(leds, count, pixel, width, color);
}

void drawMappedSections(
    CRGB *leds,
    uint16_t count,
    const StripSection *sections,
    uint8_t sectionCount,
    uint16_t firstVisiblePixel)
{
  fill_solid(leds, count, CRGB::Black);

  uint16_t startPixel = 0;
  for (uint8_t sectionIndex = 0; sectionIndex < sectionCount; sectionIndex++)
  {
    const StripSection &section = sections[sectionIndex];
    const CRGB color = sectionColor(sectionIndex, section.orientation);
    uint16_t sectionEnd = startPixel + section.length;
    if (sectionEnd > count)
    {
      sectionEnd = count;
    }

    const uint16_t visibleStart = (startPixel < firstVisiblePixel) ? firstVisiblePixel : startPixel;

    for (uint16_t pixel = visibleStart; pixel < sectionEnd; pixel++)
    {
      leds[pixel] = color;
    }

    for (uint16_t pixel = startPixel; pixel < sectionEnd; pixel += GRID_SPACING)
    {
      if (pixel >= firstVisiblePixel)
      {
        leds[pixel] = CRGB::White;
      }
    }

    if (sectionEnd > startPixel)
    {
      drawMarkerAfter(leds, count, startPixel, BOUNDARY_WIDTH, CRGB::White, firstVisiblePixel);
      drawMarkerAfter(leds, count, sectionEnd - 1, BOUNDARY_WIDTH, CRGB::White, firstVisiblePixel);
    }

    startPixel = sectionEnd;
  }

  drawMarkerAfter(
      leds,
      count,
      totalMappedLength(sections, sectionCount),
      NEXT_SECTION_MARKER_WIDTH,
      CRGB::Blue,
      firstVisiblePixel);
}

void setup()
{
  FastLED.addLeds<WS2812B, LED_PIN_1, COLOR_ORDER>(leds1, LED_COUNT_1);
  FastLED.addLeds<WS2812B, LED_PIN_2, COLOR_ORDER>(leds2, LED_COUNT_2);
  FastLED.addLeds<WS2812B, LED_PIN_3, COLOR_ORDER>(leds3, LED_COUNT_3);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);
}

void loop()
{
  for (uint8_t stripIndex = 0; stripIndex < STRIP_COUNT; stripIndex++)
  {
    const StripSectionMap &sectionMap = stripSectionMaps[stripIndex];

    if (!sectionMap.enabled || sectionMap.sectionCount == 0)
    {
      fill_solid(sectionMap.leds, sectionMap.ledCount, CRGB::Black);
    }
    else
    {
      drawMappedSections(
          sectionMap.leds,
          sectionMap.ledCount,
          sectionMap.sections,
          sectionMap.sectionCount,
          sectionMap.hideBeforePixel);
    }
  }

  FastLED.show();
  delay(50);
}
