
#include "WS2812Serial.h"

bool WS2812Serial::begin()
{
	uint32_t divisor, portconfig, hwtrigger;
	KINETISK_UART_t *uart;

	switch (pin) {
#if defined(KINETISK) // Teensy 3.x
	  case 1: // Serial1
	  case 5:
	#if defined(__MK64FX512__) || defined(__MK66FX1M0__)
	  case 26:
	#endif
		uart = &KINETISK_UART0;
		divisor = BAUD2DIV(2400000);
		portconfig = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3);
		hwtrigger = DMAMUX_SOURCE_UART0_TX;
		SIM_SCGC4 |= SIM_SCGC4_UART0;
		break;

	  case 10: // Serial2
	#if defined(__MK20DX128__) || defined(__MK20DX256__)
	  case 31:
	#elif defined(__MK64FX512__) || defined(__MK66FX1M0__)
	  case 58:
	#endif
		uart = &KINETISK_UART1;
		divisor = BAUD2DIV2(2400000);
		portconfig = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3);
		hwtrigger = DMAMUX_SOURCE_UART1_TX;
		SIM_SCGC4 |= SIM_SCGC4_UART1;
		break;

	  case 8: // Serial3
	  case 20:
		uart = &KINETISK_UART2;
		divisor = BAUD2DIV3(2400000);
		portconfig = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3);
		hwtrigger = DMAMUX_SOURCE_UART2_TX;
		SIM_SCGC4 |= SIM_SCGC4_UART2;
		break;

	#if defined(__MK64FX512__) || defined(__MK66FX1M0__)
	  case 32: // Serial4
	  case 62:
		uart = &KINETISK_UART3;
		divisor = BAUD2DIV3(2400000);
		portconfig = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3);
		hwtrigger = DMAMUX_SOURCE_UART3_TX;
		SIM_SCGC4 |= SIM_SCGC4_UART3;
		break;

	  case 33: // Serial5
		uart = &KINETISK_UART4;
		divisor = BAUD2DIV3(2400000);
		portconfig = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3);
		hwtrigger = DMAMUX_SOURCE_UART4_RXTX;
		SIM_SCGC1 |= SIM_SCGC1_UART4;
		break;
	#endif
	#if defined(__MK64FX512__)
	  case 48: // Serial6
		uart = &KINETISK_UART5;
		divisor = BAUD2DIV3(2400000);
		portconfig = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3);
		hwtrigger = DMAMUX_SOURCE_UART5_RXTX;
		SIM_SCGC1 |= SIM_SCGC1_UART5;
		break;
	#endif

#elif defined(KINETISL)	// Teensy LC
	  case 1: // Serial1
	  case 5:
		uart = &KINETISK_UART0;
		divisor = 2;
		portconfig = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3);
		hwtrigger = DMAMUX_SOURCE_UART0_TX;
		SIM_SCGC4 |= SIM_SCGC4_UART0;
		break;
	  case 4:
		uart = &KINETISK_UART0;
		divisor = 2;
		portconfig = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(2);
		hwtrigger = DMAMUX_SOURCE_UART0_TX;
		SIM_SCGC4 |= SIM_SCGC4_UART0;
		break;
	  case 24:
		uart = &KINETISK_UART0;
		divisor = 2;
		portconfig = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(4);
		hwtrigger = DMAMUX_SOURCE_UART0_TX;
		SIM_SCGC4 |= SIM_SCGC4_UART0;
		break;
#endif
	  default:
		return false; // pin not supported
	}
	if (!dma) {
		dma = new DMAChannel;
		if (!dma) return false; // unable to allocate DMA channel
	}
#if defined(KINETISK)
	uart->BDH = (divisor >> 13) & 0x1F;
	uart->BDL = (divisor >> 5) & 0xFF;
	uart->C4 = divisor & 0x1F;
#elif defined(KINETISL)
	uart->BDH = (divisor >> 8) & 0x1F;
	uart->BDL = divisor & 0xFF;
	uart->C4 = 9;
#endif
	uart->C1 = 0;
	uart->C2 = UART_C2_TE | UART_C2_TIE;
	uart->C3 = UART_C3_TXINV;
	uart->C5 = UART_C5_TDMAS;
#if defined(KINETISK)
	uart->PFIFO = 0; // TODO: is this ok for Serial3-6?
#endif
	*(portConfigRegister(pin)) = portconfig;
	dma->destination(uart->D);
	dma->triggerAtHardwareEvent(hwtrigger);
	memset(drawBuffer, 0, numled * 3);
	return true;
}

void WS2812Serial::show()
{
	// wait if prior DMA still in progress
#if defined(KINETISK)
	while ((DMA_ERQ & (1 << dma->channel))) {
		yield();
	}
#elif defined(KINETISL)
	while ((dma->CFG->DCR & DMA_DCR_ERQ)) {
		yield();
	}
#elif defined(KINETISL)
#endif
	// wait 300us WS2812 reset time
	uint32_t min_elapsed = ((numled * 34134) >> 10) + 300;
	if (min_elapsed < 2500) min_elapsed = 2500;
	uint32_t m;
	while (1) {
		m = micros();
		if ((m - prior_micros) > min_elapsed) break;
		yield();
	}
	prior_micros = m;
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

