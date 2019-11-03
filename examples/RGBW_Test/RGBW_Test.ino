/* WS2812Serial RGBWtest Example

   Test LEDs by turning then 7 different colors.

   This example code is in the public domain. */

#include <WS2812Serial.h>

const int numled = 64;
const int pin = 1;

// Usable pins:
//   Teensy LC:   1, 4, 5, 24
//   Teensy 3.2:  1, 5, 8, 10, 31   (overclock to 120 MHz for pin 8)
//   Teensy 3.5:  1, 5, 8, 10, 26, 32, 33, 48
//   Teensy 3.6:  1, 5, 8, 10, 26, 32, 33

byte drawingMemory[numled*4];         //  4 bytes per LED
DMAMEM byte displayMemory[numled*16]; // 16 bytes per LED

WS2812Serial leds(numled, displayMemory, drawingMemory, pin, WS2812_RGBW);

#define RED    0x00FF0000
#define GREEN  0x0000FF00
#define BLUE   0x000000FF
#define YELLOW 0x00FFFF00
#define PINK   0x00FF1088
#define ORANGE 0x00E05800
#define WHITE  0xFF000000

// Less intense...
/*
#define RED    0x00160000
#define GREEN  0x00001600
#define BLUE   0x00000016
#define YELLOW 0x00101400
#define PINK   0x00120009
#define ORANGE 0x00100400
#define WHITE  0x16000000
*/

void setup() {
  leds.begin();
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

void colorWipe(int color, int wait) {
  for (int i=0; i < leds.numPixels(); i++) {
    leds.setPixel(i, color);
    leds.show();
    delayMicroseconds(wait);
  }
}
