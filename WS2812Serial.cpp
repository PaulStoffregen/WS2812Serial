
#include "WS2812Serial.h"

void WS2812Serial::begin()
{
	if (!uart) return;
	if (!dma) dma = new DMAChannel;
	if (!dma) return;

	SIM_SCGC4 |= SIM_SCGC4_UART0;
	uint32_t divisor = BAUD2DIV(2400000);

	CORE_PIN1_CONFIG = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3);

        UART0_BDH = (divisor >> 13) & 0x1F;
        UART0_BDL = (divisor >> 5) & 0xFF;
        UART0_C4 = divisor & 0x1F;

	UART0_C1 = 0;
	UART0_PFIFO = 0;
	UART0_C2 = UART_C2_TE;
	UART0_C3 = UART_C3_TXINV;
	UART0_C5 = UART_C5_TDMAS;

	memset(drawBuffer, 0, numled * 3);
}

void WS2812Serial::show()
{

	//UART0_D = 0xDB;
	//UART0_D = 0xDA;
	//UART0_D = 0xDB;
	
	const uint8_t *p = drawBuffer;
	const uint8_t *end = p + (numled * 3);
	uint8_t *fb = frameBuffer;
	while (p < end) {
		uint8_t b = *p++;
		uint8_t g = *p++;
		uint8_t r = *p++;
		uint32_t n=0;
		switch (config) {
		  case WS2812_RGB: n = (r << 16) | (g << 8) | b; break;
		  case WS2812_RBG: n = (r << 16) | (b << 8) | g; break;
		  case WS2812_GRB: n = (g << 16) | (r << 8) | b; break;
		  case WS2812_GBR: n = (g << 16) | (b << 8) | r; break;
		  case WS2812_BRG: n = (b << 16) | (r << 8) | g; break;
		  case WS2812_BGR: n = (b << 16) | (g << 8) | r; break;
		}
		//Serial.printf("n = %06X\n", n);
		const uint8_t *stop = fb + 8;
		do {
			uint8_t x = 0x92;
			if (!(n & 0x00800000)) x |= 0x01;
			if (!(n & 0x00400000)) x |= 0x04;
			if (!(n & 0x00200000)) x |= 0x20;
			n <<= 3;
			*fb++ = x;
		} while (fb < stop);
	}

	dma->sourceBuffer(frameBuffer, numled * 8);
	dma->destination(UART0_D);
	dma->transferSize(1);
	dma->transferCount(numled * 8);
	dma->disableOnCompletion();
	dma->triggerAtHardwareEvent(DMAMUX_SOURCE_UART0_TX);
	dma->enable();
	UART0_C2 = UART_C2_TE | UART_C2_TIE;
}


