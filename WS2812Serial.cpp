
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

	frameBuffer[0] = 0xDB;
	frameBuffer[1] = 0xDA;
	frameBuffer[2] = 0xDB;

	dma->sourceBuffer(frameBuffer, 3);
	dma->destination(UART0_D);
	dma->transferSize(1);
	dma->transferCount(3);
	dma->disableOnCompletion();

	dma->triggerAtHardwareEvent(DMAMUX_SOURCE_UART0_TX);
}

void WS2812Serial::show()
{

	//UART0_D = 0xDB;
	//UART0_D = 0xDA;
	//UART0_D = 0xDB;
	
	dma->enable();
	UART0_C2 = UART_C2_TE | UART_C2_TIE;

}


