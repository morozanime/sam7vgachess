/*
 * ps2mouse.c
 *
 *  Created on: 3 рту. 2019 у.
 *      Author: URA
 */

#include "main.h"
#include "ps2mouse.h"

#define DATA_mask	(1<<5)
#define CLOCK_mask	(1<<7)
#define	DATA	{DATA_mask, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_PULLUP | PIO_OPENDRAIN}
#define	CLOCK	{CLOCK_mask, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_PULLUP | PIO_OPENDRAIN}
static const Pin pins[] = { DATA, CLOCK };

void ps2Init(void) {
	AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_PIOA;
	PIO_Configure(pins, PIO_LISTSIZE(pins));
	AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_TC1;
	TC_Configure(AT91C_BASE_TC1, 0);
	TC_Start(AT91C_BASE_TC1);
	AT91C_BASE_PIOA->PIO_SODR = DATA_mask | CLOCK_mask;
}

void ps2DelayUs(int us) {
	uint32_t ticks = (BOARD_MCK / 2 / 10000) * us / 100;
	uint32_t elapsed = 0;
	uint16_t last = AT91C_BASE_TC1->TC_CV;
	uint16_t t, dt;
	while (elapsed < ticks) {
		t = AT91C_BASE_TC1->TC_CV;
		dt = t - last;
		last = t;
		elapsed += dt;
	}
}

void ps2Write(uint8_t byte) {
	AT91C_BASE_PIOA->PIO_SODR = DATA_mask | CLOCK_mask;
	ps2DelayUs(300);
	AT91C_BASE_PIOA->PIO_CODR = CLOCK_mask;
	ps2DelayUs(300);
	AT91C_BASE_PIOA->PIO_CODR = DATA_mask;
	ps2DelayUs(10);
	AT91C_BASE_PIOA->PIO_SODR = CLOCK_mask;
	ps2DelayUs(10);
	while (AT91C_BASE_PIOA->PIO_PDSR & CLOCK_mask)
		;
	uint8_t d = byte;
	for (int i = 0; i < 8; i++) {
		if (d & 1)
			AT91C_BASE_PIOA->PIO_SODR = DATA_mask;
		else
			AT91C_BASE_PIOA->PIO_CODR = DATA_mask;
		while ((AT91C_BASE_PIOA->PIO_PDSR & CLOCK_mask) == 0)
			;
		while ((AT91C_BASE_PIOA->PIO_PDSR & CLOCK_mask))
			;
		d >>= 1;
	}
	if ((byte ^ (byte >> 4) ^ (byte >> 2) ^ (byte >> 1)) & 1)
		AT91C_BASE_PIOA->PIO_SODR = DATA_mask;
	else
		AT91C_BASE_PIOA->PIO_CODR = DATA_mask;
	while (!(AT91C_BASE_PIOA->PIO_PDSR & CLOCK_mask))
		;
	while ((AT91C_BASE_PIOA->PIO_PDSR & CLOCK_mask))
		;
	AT91C_BASE_PIOA->PIO_SODR = DATA_mask;
	ps2DelayUs(50);
	while ((AT91C_BASE_PIOA->PIO_PDSR & CLOCK_mask))
		;
	while (!(AT91C_BASE_PIOA->PIO_PDSR & CLOCK_mask) || !(AT91C_BASE_PIOA->PIO_PDSR & DATA_mask))
		;
	AT91C_BASE_PIOA->PIO_CODR = CLOCK_mask;
}

uint8_t ps2Read(void) {
	AT91C_BASE_PIOA->PIO_SODR = DATA_mask | CLOCK_mask;
	ps2DelayUs(50);
	while ((AT91C_BASE_PIOA->PIO_PDSR & CLOCK_mask))
		;
	ps2DelayUs(5);
	while (!(AT91C_BASE_PIOA->PIO_PDSR & CLOCK_mask))
		;
	uint8_t byte;
	for (int i = 0; i < 8; i++) {
		while ((AT91C_BASE_PIOA->PIO_PDSR & CLOCK_mask))
			;
		byte >>= 1;
		if (AT91C_BASE_PIOA->PIO_PDSR & DATA_mask)
			byte |= 0x80;
		while (!(AT91C_BASE_PIOA->PIO_PDSR & CLOCK_mask))
			;
	}
	while ((AT91C_BASE_PIOA->PIO_PDSR & CLOCK_mask))
		;
	while (!(AT91C_BASE_PIOA->PIO_PDSR & CLOCK_mask))
		;
	while ((AT91C_BASE_PIOA->PIO_PDSR & CLOCK_mask))
		;
	while (!(AT91C_BASE_PIOA->PIO_PDSR & CLOCK_mask))
		;
	AT91C_BASE_PIOA->PIO_CODR = CLOCK_mask;
	return byte;
}
