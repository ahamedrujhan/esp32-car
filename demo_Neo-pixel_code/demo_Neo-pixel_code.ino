#include <Adafruit_NeoPixel.h>

#define PIN_PIXELS 48 // GPIO pin connected to NeoPixel data
#define NUM_PIXELS 1 // Number of NeoPixels in your strip

Adafruit_NeoPixel strip(NUM_PIXELS, PIN_PIXELS, NEO_GRB + NEO_KHZ800); // NEO_GRB for standard WS2812B

void setup() {
  strip.begin(); // Initialize NeoPixel strip
  strip.show();  // Turn all pixels off
  strip.setBrightness(50); // Set brightness (0-255)
}

void loop() {
  colorWipe(strip.Color(255, 0, 0), 50); // Red color wipe
  colorWipe(strip.Color(0, 255, 0), 50); // Green color wipe
  colorWipe(strip.Color(0, 0, 255), 50); // Blue color wipe
}

void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color); // Set pixel color
    strip.show();                  // Update strip
    delay(wait);
  }
}
