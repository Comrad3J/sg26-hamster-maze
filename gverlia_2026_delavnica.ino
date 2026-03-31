#include <FastLED.h>

#define LED_PIN     6
#define NUM_LEDS    600
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB

#define NUM_CROSS   6
#define NUM_FOOD    8   // number of food items

CRGB leds[NUM_LEDS];

// crossings
int crossA[NUM_CROSS] = {40, 140, 240, 340, 440, 580};
int crossB[NUM_CROSS] = {520, 420, 320, 220, 120,  20};

// hamster state
int pos = 0;
int dir = 1;
uint8_t hamsterBrightness = 80;

// food positions
int food[NUM_FOOD];

// timing
unsigned long lastStepTime = 0;
unsigned long stepInterval = 30;
const unsigned long FAST_STEP = 18;
const unsigned long SLOW_STEP = 90;

// behavior params
const uint8_t BRIGHTNESS_STEP = 25;
const uint8_t DECAY_STEP = 1;
const uint8_t MAX_HAMSTER_BRIGHTNESS = 250;
const uint8_t MIN_HAMSTER_BRIGHTNESS = 60;

const uint8_t TRAIL_FADE = 45;
const int SLOWDOWN_DISTANCE = 12;

// colors
const uint8_t HAMSTER_HUE = 32;  // yellow-orange
const uint8_t FOOD_HUE = 96;     // green-ish

void spawnFood() {
  for (int i = 0; i < NUM_FOOD; i++) {
    food[i] = random(NUM_LEDS);
  }
}

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  randomSeed(analogRead(A0));

  spawnFood();
}

int getCrossIndex(int p) {
  for (int i = 0; i < NUM_CROSS; i++) {
    if (p == crossA[i] || p == crossB[i]) return i;
  }
  return -1;
}

int distanceToNextCrossing(int p, int d) {
  int minDist = NUM_LEDS;

  for (int i = 0; i < NUM_CROSS; i++) {
    int targets[2] = { crossA[i], crossB[i] };

    for (int j = 0; j < 2; j++) {
      int t = targets[j];
      int dist = (d > 0) ? (t - p) : (p - t);

      if (dist >= 0 && dist < minDist) {
        minDist = dist;
      }
    }
  }
  return minDist;
}

void loop() {
  unsigned long now = millis();

  int dist = distanceToNextCrossing(pos, dir);

  if (dist <= SLOWDOWN_DISTANCE) {
    stepInterval = map(dist, 0, SLOWDOWN_DISTANCE, SLOW_STEP, FAST_STEP);
  } else {
    stepInterval = FAST_STEP;
  }

  if (now - lastStepTime < stepInterval) return;
  lastStepTime = now;

  // fade trail
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].fadeToBlackBy(TRAIL_FADE);
  }

  // draw food
  for (int i = 0; i < NUM_FOOD; i++) {
    leds[food[i]] = CHSV(FOOD_HUE, 255, 120);
  }

  // move hamster
  pos += dir;

  if (pos <= 0) { pos = 0; dir = 1; }
  if (pos >= NUM_LEDS - 1) { pos = NUM_LEDS - 1; dir = -1; }

  // crossing logic
  int ci = getCrossIndex(pos);
  if (ci >= 0) {
    if (pos == crossA[ci]) pos = crossB[ci];
    else pos = crossA[ci];

    dir = random(2) ? 1 : -1;
  }

  // eating logic
  for (int i = 0; i < NUM_FOOD; i++) {
    if (pos == food[i]) {
      hamsterBrightness = min(255, hamsterBrightness + BRIGHTNESS_STEP);

      // respawn food somewhere else
      food[i] = random(NUM_LEDS);
    }
  }

  // passive decay (prevents infinite brightness)
  if (hamsterBrightness > MIN_HAMSTER_BRIGHTNESS) {
    hamsterBrightness -= DECAY_STEP;
  }

  // draw hamster
  leds[pos] = CHSV(HAMSTER_HUE, 255, hamsterBrightness);

  FastLED.show();
}