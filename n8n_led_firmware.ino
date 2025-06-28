// Firmware for 3D backlit n8n logo (Controller: XIAO SAMD21)

#include <Adafruit_NeoPixel.h>
#define LED_PIN     1
#define LED_COUNT   56
#define BRIGHTNESS  50 // Set BRIGHTNESS to about 1/5 (max = 255)
#define TOUCH_PIN   2  // Capacitive touch signal pin

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

bool isOn = false;
bool lastTouch = false;
bool demoMode = false;
unsigned long touchStartTime = 0;
bool longPressDetected = false;
bool touchPressed = false;
bool animationInterrupted = false;
bool demoActivated = false;

// Colors
uint32_t redColor = strip.Color(255, 0, 6);
uint32_t orangeColor = strip.Color(255, 80, 0);
uint32_t offColor = strip.Color(0, 0, 0);

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
  delay(10);
  pinMode(TOUCH_PIN, INPUT);
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS);
  delay(2000);             // Optional boot delay
}

void loop() {
  bool currentTouch = digitalRead(TOUCH_PIN);
  
  // Handle touch press detection
  if (currentTouch && !lastTouch) {
    // Touch just started
    touchStartTime = millis();
    longPressDetected = false;
    touchPressed = true;
    animationInterrupted = false;
    demoActivated = false;
  }
  
  // Handle 3-second turn-off (takes priority over demo)
  if (touchPressed && (millis() - touchStartTime >= 3000) && !longPressDetected) {
    longPressDetected = true;
    // Turn off immediately when 3 seconds reached
    turnOffInstant();
    isOn = false;
    demoMode = false;
    // Don't reset touchPressed here - let it reset on finger lift
  }
  
  // Handle 1.5-second demo activation (only if 3-second hasn't been reached)
  if (touchPressed && (millis() - touchStartTime >= 1500) && !longPressDetected && !demoActivated && !demoMode) {
    demoActivated = true; // Mark that demo should start, but don't start yet
  }
  
  // Handle touch release
  if (!currentTouch && lastTouch) {
    touchPressed = false;
    
    if (longPressDetected) {
      // Was a long press (3+ seconds) - already handled above
      longPressDetected = false;
    } else if (demoActivated) {
      // Was held for 1.5+ seconds but less than 3 - start demo now
      demoMode = true;
      demoActivated = false;
      animationInterrupted = false;
    } else {
      // Was a short press
      animationInterrupted = false;
      
      if (demoMode) {
        // Exit demo mode - set all LEDs to red
        demoMode = false;
        setAllLEDs(redColor);
        isOn = true;
      } else {
        if (!isOn) {
          // Device is off, turn on with initial red animation
          animateOn(redColor, 40);
          if (!animationInterrupted) {
            isOn = true;
          }
        } else {
          // Device is on, do pattern turn-off then turn on again
          patternTurnOffAnimation(40);
        }
      }
    }
  }
  
  // Run demo mode
  if (demoMode) {
    runDemoMode();
  }
  
  lastTouch = currentTouch;
  delay(50); // Simple debounce
}

// Turn on LEDs one after another with a color (0 to 55)
void animateOn(uint32_t color, int wait) {
  for (int i = 0; i < strip.numPixels(); i++) {
    // Check for interruption
    if (checkForInterrupt()) {
      // Animation was interrupted - LEDs stay as they are
      // Start pattern turn-off immediately
      patternTurnOffOnly(30);
      return;
    }
    
    strip.setPixelColor(i, color);
    strip.show();
    delay(wait);
  }
}

// Pattern turn-off animation using LED groups, then turn on again
void patternTurnOffAnimation(int wait) {
  // Turn off LEDs in groups according to the pattern
  patternTurnOffOnly(wait);
  
  // Wait 1 second
  if (!waitWithButtonCheck(1000)) {
    return; // Interrupted during wait
  }
  
  // Turn on from beginning to end (0 to 55)
  for (int i = 0; i < strip.numPixels(); i++) {
    if (checkForInterrupt()) {
      // If interrupted during turn-on, start pattern turn-off again
      patternTurnOffOnly(wait);
      return;
    }
    
    strip.setPixelColor(i, redColor);
    strip.show();
    delay(wait);
  }
}

// Pattern turn-off only (no turn-on after)
void patternTurnOffOnly(int wait) {
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

// Walking trail animation - LED walks and leaves trail at end positions
void walkingTrailAnimation(uint32_t walkColor, uint32_t bgColor, int wait) {
  int permanentPositions[LED_COUNT]; // Track which positions should stay in walkColor
  
  // Initialize all positions to background color
  for (int i = 0; i < strip.numPixels(); i++) {
    permanentPositions[i] = 0; // 0 = bgColor, 1 = walkColor
    strip.setPixelColor(i, bgColor);
  }
  strip.show();
  
  // Each pass goes one position less (55, 54, 53, ..., 0)
  for (int pass = 0; pass < strip.numPixels(); pass++) {
    int endPosition = strip.numPixels() - 1 - pass; // 55, 54, 53, ..., 0
    
    // Walk from position 0 to endPosition
    for (int pos = 0; pos <= endPosition; pos++) {
      // Check for interruption
      if (checkForDemoInterrupt()) {
        return;
      }
      
      // Set all LEDs based on their state
      for (int i = 0; i < strip.numPixels(); i++) {
        if (i == pos) {
          // Current walking position - always walkColor
          strip.setPixelColor(i, walkColor);
        } else if (permanentPositions[i] == 1) {
          // Permanent walkColor position
          strip.setPixelColor(i, walkColor);
        } else {
          // Background color
          strip.setPixelColor(i, bgColor);
        }
      }
      
      strip.show();
      delay(wait);
      
      // If we reached the end position, mark it as permanent
      if (pos == endPosition) {
        permanentPositions[endPosition] = 1;
      }
    }
  }
}

void runDemoMode() {
  while (demoMode) {
    // 1. Orange walking trail animation
    walkingTrailAnimation(orangeColor, redColor, 70);
    if (!demoMode) return;
    
    // 2. Wait 2 seconds
    if (!waitWithButtonCheck(2000)) return;
    
    // 3. Red walking trail animation (replaces orange)
    walkingTrailAnimation(redColor, orangeColor, 70);
    if (!demoMode) return;
    
    // 4. Wait 2 seconds before next loop
    if (!waitWithButtonCheck(2000)) return;
  }
}

bool checkForInterrupt() {
  bool currentTouch = digitalRead(TOUCH_PIN);
  
  // Handle new touch press during normal animations
  if (currentTouch && !lastTouch) {
    touchStartTime = millis();
    longPressDetected = false;
    touchPressed = true;
    animationInterrupted = true;
    
    // Don't change LED state - just return to interrupt the current animation
    return true;
  }
  
  // Check for 3-second turn-off during animations
  if (touchPressed && (millis() - touchStartTime >= 3000) && !longPressDetected) {
    longPressDetected = true;
    turnOffInstant();
    isOn = false;
    return true;
  }
  
  lastTouch = currentTouch;
  return false;
}

bool checkForDemoInterrupt() {
  bool currentTouch = digitalRead(TOUCH_PIN);
  
  // Check for button press during demo
  if (currentTouch && !lastTouch) {
    demoMode = false;
    //setAllLEDs(redColor);
    isOn = true;
    return true;
  }
  
  lastTouch = currentTouch;
  return false;
}

bool waitWithButtonCheck(int milliseconds) {
  unsigned long startTime = millis();
  while (millis() - startTime < milliseconds) {
    if (demoMode) {
      if (checkForDemoInterrupt()) {
        return false; // Interrupted
      }
    } else {
      if (checkForInterrupt()) {
        return false; // Interrupted
      }
    }
    delay(50);
  }
  return true; // Completed without interruption
}

void setAllLEDs(uint32_t color) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

void turnOffInstant() {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, offColor);
  }
  strip.show();
}
