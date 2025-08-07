// Firmware for 3D backlit n8n logo (Controller: XIAO ESP32C3)
#include <Adafruit_NeoPixel.h>
#define LED_PIN     3
#define TOUCH_PIN   4     // Capacitive touch signal pin
#define LED_COUNT   56
#define BRIGHTNESS  50    // Set BRIGHTNESS to about 1/5 (max = 255)

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
bool isOn = false;
bool lastTouch = false;

// Colors
uint32_t redColor;
uint32_t offColor;

// LED groups for pattern turn-off (0-based indexing)
const int ledGroups[][5] = {
  {0, 9, -1, -1, -1},      // 1, 10
  {1, 8, -1, -1, -1},      // 2, 9
  {2, 7, -1, -1, -1},      // 3, 8
  {3, 6, -1, -1, -1},      // 4, 7
  {4, 5, -1, -1, -1},      // 5, 6
  {10, -1, -1, -1, -1},    // 11
  {11, -1, -1, -1, -1},    // 12
  {12, 21, -1, -1, -1},    // 13, 22
  {13, 20, -1, -1, -1},    // 14, 21
  {14, 19, -1, -1, -1},    // 15, 20
  {15, 18, -1, -1, -1},    // 16, 19
  {16, 17, -1, -1, -1},    // 17, 18
  {22, -1, -1, -1, -1},    // 23
  {23, -1, -1, -1, -1},    // 24
  {24, 41, -1, -1, -1},    // 25, 42
  {25, 42, -1, -1, -1},    // 26, 43
  {26, 43, -1, -1, -1},    // 27, 44
  {27, 44, -1, -1, -1},    // 28, 45
  {28, 45, -1, -1, -1},    // 29, 46
  {29, 46, 55, -1, -1},    // 30, 47, 56
  {30, 47, 54, -1, -1},    // 31, 48, 55
  {31, 40, 48, 53, -1},    // 32, 41, 49, 54
  {32, 39, 49, 52, -1},    // 33, 40, 50, 53
  {33, 38, 50, 51, -1},    // 34, 39, 51, 52
  {34, 37, -1, -1, -1},    // 35, 38
  {35, 36, -1, -1, -1}     // 36, 37
};
const int numGroups = 26;

// Function declarations
void animateOn(uint32_t color, int wait);
void patternTurnOff(int wait);

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(TOUCH_PIN, INPUT);
  
  strip.begin(); // INITIALIZE NeoPixel strip object
  strip.show(); // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS);
  
  // Initialize colors
  redColor = strip.Color(255, 0, 6);
  offColor = strip.Color(0, 0, 0);
  
  delay(2000); // Optional boot delay
}

void loop() {
  // Read touch sensor (using digital read for simplicity)
  bool currentTouch = digitalRead(TOUCH_PIN);
  
  if (!currentTouch && lastTouch) {
    if (!isOn) {
      animateOn(redColor, 40);
      isOn = true;
    } else {
      patternTurnOff(40);
      isOn = false;
    }
  }
  
  lastTouch = currentTouch;
  delay(50); // Simple debounce
}

// Turn on LEDs one after another with a color (0 to 55)
void animateOn(uint32_t color, int wait) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(wait);
  }
}

// Pattern turn-off animation using LED groups
void patternTurnOff(int wait) {
  for (int group = 0; group < numGroups; group++) {
    // Turn off all LEDs in this group
    for (int j = 0; j < 5; j++) {
      int ledIndex = ledGroups[group][j];
      if (ledIndex >= 0) { // -1 indicates end of group
        strip.setPixelColor(ledIndex, offColor);
      }
    }
    strip.show();
    delay(wait);
  }
}
