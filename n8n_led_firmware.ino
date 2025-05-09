// Firmware for 3D backlit n8n logo (Controller: XIAO SAMD21)

#include <Adafruit_NeoPixel.h>

#define LED_PIN     1
#define LED_COUNT   56
#define BRIGHTNESS  50 // Set BRIGHTNESS to about 1/5 (max = 255)
#define TOUCH_PIN   2  // Capacitive touch signal pin

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

bool isOn = false;
bool lastTouch = false;

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

  if (currentTouch && !lastTouch) { // Rising edge detected
    if (isOn) {
      turnOff();
      isOn = false;
    } else {
      animateLinearly(strip.Color(255, 0, 6), 40);
      isOn = true;
    }
  }

  lastTouch = currentTouch;
  delay(50); // Simple debounce
}

// Fill strip pixels one after another with a color
void animateLinearly(uint32_t color, int wait) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(wait);
  }
}

void turnOff() {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();
}
