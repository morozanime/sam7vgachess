/*
 * main.h
 *
 *  Created on: 27 èþë. 2019 ã.
 *      Author: URA
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <board.h>
#include <spi/spi.h>
#include <pio/pio.h>
#include <pio/pio_it.h>
#if defined(AT91C_BASE_PITC)
#include <pit/pit.h>
#endif
#include <irq/irq.h>
#include <tc/tc.h>
//#include <utility/led.h>
#include <utility/trace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(cortexm3)
#include <systick/systick.h>
#endif

//typedef unsigned char uint8_t;
#define	uint8_t unsigned char
#define	uint16_t unsigned short
#define	uint32_t unsigned int
#define	int8_t signed char
#define	int16_t signed short
#define	int32_t signed int

#define	LCD_RESET	{1 << 14, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
#define	LCD_A0		{1 << 9, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_0, PIO_DEFAULT}
#define	LCD_LED		{1 << 8, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_0, PIO_DEFAULT}
#define	PINS_LCD	LCD_RESET, LCD_A0, LCD_LED

#define	MISO_SPI0	{1 << 16, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define	MOSI_SPI0	{1 << 17, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define	SCK_SPI0	{1 << 18, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define	SS_SPI0		{1 << 15, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define	PINS_SPI0	MISO_SPI0, MOSI_SPI0, SCK_SPI0, SS_SPI0

//#define	SS_SPI1		{1 << 21, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
//#define	SCK_SPI1	{1 << 22, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
#define	MOSI_SPI1	{1 << 23, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
//#define	MISO_SPI1	{1 << 24, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
//#define	PINS_SPI1	MISO_SPI1, MOSI_SPI1, SCK_SPI1, SS_SPI1
#define	PINS_SPI1	MOSI_SPI1

extern const Pin pinLCD_RESET;
extern const Pin pinLCD_A0;
extern const Pin pinLCD_LED;

#define	pgm_read_word(x)	(*(x))
//#define	pgm_read_byte(x)	(*(x))
#define	LCD_A0_SetLow()		PIO_Clear(&pinLCD_A0)
#define	LCD_A0_SetHigh()	PIO_Set(&pinLCD_A0)
#define	LCD_RESET_SetLow()	PIO_Clear(&pinLCD_RESET)
#define	LCD_RESET_SetHigh()	PIO_Set(&pinLCD_RESET)
#define	LCD_LED_A_SetLow()	PIO_Clear(&pinLCD_LED)
#define	LCD_LED_A_SetHigh()	PIO_Set(&pinLCD_LED)

void _delay_ms(unsigned long delay);

extern uint8_t textBuff[480 / 8][640 / 8];

#endif /* MAIN_H_ */
