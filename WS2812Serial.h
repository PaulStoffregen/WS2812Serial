#ifndef WS2812Serial_h_
#define WS2812Serial_h_

#include <Arduino.h>
#include "DMAChannel.h"

#define WS2812_RGB      0       // The WS2811 datasheet documents this way
#define WS2812_RBG      1
#define WS2812_GRB      2       // Most LED strips are wired this way
#define WS2812_GBR      3
#define WS2812_BRG      4
#define WS2812_BGR      5

class WS2812Serial {
public:
	constexpr WS2812Serial(uint16_t num, void *fb, void *db, uint8_t pin, uint8_t cfg) :
		numled(num), pin(pin), config(cfg),
		frameBuffer((uint8_t *)fb), drawBuffer((uint8_t *)db) {
	}
	bool begin();
	void setPixel(uint32_t num, int color) {
		if (num >= numled) return;
		num *= 3;
		drawBuffer[num+0] = color & 255;
		drawBuffer[num+1] = (color >> 8) & 255;
		drawBuffer[num+2] = (color >> 16) & 255;
	}
	void setPixel(uint32_t num, uint8_t red, uint8_t green, uint8_t blue) {
		if (num >= numled) return;
		num *= 3;
		drawBuffer[num+0] = blue;
		drawBuffer[num+1] = green;
		drawBuffer[num+2] = red;
	}
	void show();
	bool busy();
	uint16_t numPixels() {
		return numled;
	}
private:
	const uint16_t numled;
	const uint8_t pin;
	const uint8_t config;
	uint8_t *frameBuffer;
	uint8_t *drawBuffer;
	DMAChannel *dma = nullptr;
	uint32_t prior_micros = 0;
};

#endif
