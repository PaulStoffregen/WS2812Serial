
#include "WS2812Serial.h"

bool WS2812Serial::begin()
{
	uint32_t divisor, portconfig, hwtrigger;
	KINETISK_UART_t *uart;

	switch (pin) {
	  case 1:
		uart = &KINETISK_UART0;
		divisor = BAUD2DIV(2400000);
		portconfig = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3);
		hwtrigger = DMAMUX_SOURCE_UART0_TX;
		SIM_SCGC4 |= SIM_SCGC4_UART0;
		break;
	  default:
		return false; // pin not supported
	}
	if (!dma) {
		dma = new DMAChannel;
		if (!dma) return false; // unable to allocate DMA channel
	}
	uart->BDH = (divisor >> 13) & 0x1F;
	uart->BDL = (divisor >> 5) & 0xFF;
	uart->C4 = divisor & 0x1F;
	uart->C1 = 0;
	uart->PFIFO = 0;
	uart->C2 = UART_C2_TE | UART_C2_TIE;
	uart->C3 = UART_C3_TXINV;
	uart->C5 = UART_C5_TDMAS;
	*(portConfigRegister(pin)) = portconfig;
	dma->destination(uart->D);
	dma->triggerAtHardwareEvent(hwtrigger);
	memset(drawBuffer, 0, numled * 3);
	return true;
}

void WS2812Serial::show()
{
	// TODO: wait if prior DMA in progress
	// TODO: wait 300us WS2812 reset time
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
	dma->transferSize(1);
	dma->transferCount(numled * 8);
	dma->disableOnCompletion();
	dma->enable();
}

