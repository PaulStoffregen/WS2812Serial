/* WS2812Serial BasicTest_RGBW Example - Works with SK6812 RGBW LEDs

   Test LEDs by turning then 7 different colors.

   This example code is in the public domain. */

#include <WS2812Serial.h>

const int numled = 20;
const int pin = 1;

// Usable pins:
//   Teensy LC:   1, 4, 5, 24
//   Teensy 3.2:  1, 5, 8, 10, 31   (overclock to 120 MHz for pin 8)
//   Teensy 3.5:  1, 5, 8, 10, 26, 32, 33, 48
//   Teensy 3.6:  1, 5, 8, 10, 26, 32, 33
//   Teensy 4.0:  1, 8, 14, 17, 20, 24, 29, 39

byte drawingMemory[numled*4];         //  4 bytes per LED for RGBW
DMAMEM byte displayMemory[numled*16]; // 16 bytes per LED for RGBW

WS2812Serial leds(numled, displayMemory, drawingMemory, pin, WS2812_GRBW);

#define RED    0x00FF0000
#define GREEN  0x0000FF00
#define BLUE   0x000000FF
#define YELLOW 0x00FFD000
#define PINK   0x44F00080
#define ORANGE 0x00FF4200
#define WHITE  0xAA000000

void setup() {
  leds.begin();
  leds.setBrightness(200); // 0=off, 255=brightest
}

void loop() {
  // change all the LEDs in 1.5 seconds
  int microsec = 1500000 / leds.numPixels();

  colorWipe(RED, microsec);
  colorWipe(GREEN, microsec);
  colorWipe(BLUE, microsec);
  colorWipe(YELLOW, microsec);
  colorWipe(PINK, microsec);
  colorWipe(ORANGE, microsec);
  colorWipe(WHITE, microsec);
}

void colorWipe(int color, int wait_us) {
  for (int i=0; i < leds.numPixels(); i++) {
    leds.setPixel(i, color);
    leds.show();
    delayMicroseconds(wait_us);
  }
}
