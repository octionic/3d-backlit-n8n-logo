// Firmware for 3D backlit n8n logo (Controller: XIAO ESP32C3)
#include <Adafruit_NeoPixel.h>
#define LED_PIN     3
#define TOUCH_PIN   4
#define LED_COUNT   56
#define BRIGHTNESS  50
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

bool isOn = false;
bool lastTouch = false;
bool demoMode = false;
unsigned long touchStartTime = 0;
bool touchPressed = false;

// Colors
uint32_t redColor;
uint32_t orangeColor;
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

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(TOUCH_PIN, INPUT);
  
  strip.begin();
  strip.show();
  strip.setBrightness(BRIGHTNESS);
  
  // Initialize colors
  redColor = strip.Color(255, 0, 6);
  orangeColor = strip.Color(255, 80, 0);
  offColor = strip.Color(0, 0, 0);
  
  delay(2000);
}

void loop() {
  bool currentTouch = digitalRead(TOUCH_PIN);
  
  // Handle touch press detection
  if (currentTouch && !lastTouch) {
    touchStartTime = millis();
    touchPressed = true;
  }

  // Handle touch release
  if (!currentTouch && lastTouch) {
    if (touchPressed) {
      unsigned long pressDuration = millis() - touchStartTime;
      
      if (pressDuration >= 1500) {
        // Long press - activate demo mode
        demoMode = true;
        isOn = true;
      } else {
        // Short press - toggle normal operation
        if (!isOn) {
          animateOn(redColor, 40);
          isOn = true;
        } else {
          patternTurnOff(40);
          isOn = false;
        }
      }
    }
    touchPressed = false;
  }

  // Run demo mode if active
  if (demoMode) {
    runDemoMode();
  }

  lastTouch = currentTouch;
  delay(50);
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

void walkingTrailAnimation(uint32_t walkColor, uint32_t bgColor, int wait) {
  int permanentPositions[LED_COUNT] = {0};
  
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, bgColor);
  }
  strip.show();

  for (int pass = 0; pass < strip.numPixels(); pass++) {
    int endPosition = strip.numPixels() - 1 - pass;
    
    for (int pos = 0; pos <= endPosition; pos++) {
      if (checkDemoInterrupt()) return;
      
      for (int i = 0; i < strip.numPixels(); i++) {
        if (i == pos) {
          strip.setPixelColor(i, walkColor);
        } else if (permanentPositions[i] == 1) {
          strip.setPixelColor(i, walkColor);
        } else {
          strip.setPixelColor(i, bgColor);
        }
      }
      strip.show();
      delay(wait);
      
      if (pos == endPosition) {
        permanentPositions[endPosition] = 1;
      }
    }
  }
}

void runDemoMode() {
  while (demoMode) {
    walkingTrailAnimation(orangeColor, redColor, 70);
    if (!demoMode) {
      setAllLEDs(redColor);
      return;
    }
    
    delay(2000);
    
    walkingTrailAnimation(redColor, orangeColor, 70);
    if (!demoMode) {
      setAllLEDs(redColor);
      return;
    }
    
    delay(2000);
  }
}

bool checkDemoInterrupt() {
  bool currentTouch = digitalRead(TOUCH_PIN);
  if (currentTouch && !lastTouch) {
    demoMode = false;
    setAllLEDs(redColor);
    isOn = true;
    lastTouch = currentTouch;
    return true;
  }
  lastTouch = currentTouch;
  return false;
}

void setAllLEDs(uint32_t color) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}
