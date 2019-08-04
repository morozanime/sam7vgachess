/*
 * vga.c
 *
 *  Created on: 4 рту. 2019 у.
 *      Author: URA
 */

#include "common.h"
#include "vga.h"
#include <pio/pio.h>
#include <tc/tc.h>
#include <irq/irq.h>
#include <spi/spi.h>
#include "chess48.h"
#include "8X8WIN1251.h"

#define	VGA_VSYNC_MASK	(1 << 25)
#define	VGA_DEBUG_MASK	(1 << 24)
#define	VGA_HSYNC	{1 << 23, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_PERIPH_A, PIO_DEFAULT}
#define	VGA_VSYNC	{VGA_VSYNC_MASK, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_1, PIO_DEFAULT}
#define	VGA_VSYNC_LOW	AT91C_BASE_PIOB->PIO_CODR = VGA_VSYNC_MASK
#define	VGA_VSYNC_HIGH	AT91C_BASE_PIOB->PIO_SODR = VGA_VSYNC_MASK
#define	MOSI_SPI1	{1 << 23, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
#define	VGA_DEBUG		{VGA_DEBUG_MASK, AT91C_BASE_PIOB, AT91C_ID_PIOB, PIO_OUTPUT_0, PIO_DEFAULT}//for debug only
#define	VGA_DEBUG_LOW	AT91C_BASE_PIOB->PIO_CODR = VGA_DEBUG_MASK
#define	VGA_DEBUG_HIGH	AT91C_BASE_PIOB->PIO_SODR = VGA_DEBUG_MASK
#define	SPI_CLOCK_DIV	3	// BOARD_MCK / 3 = 25MHz - pixel clock
const Pin pinsVGA[] = { VGA_HSYNC, MOSI_SPI1, VGA_VSYNC, VGA_DEBUG };

#define	MOUSE_CURSOR_WIDTH_PX	16
#define	MOUSE_CURSOR_HEIGHT_PX	23

const uint8_t mouseCursor[] = {		//
		//
				0x00, 0x00, 0x00, 0xe0, //
				0x00, 0x40, 0x00, 0xf0, //
				0x00, 0x60, 0x00, 0xf8, //
				0x00, 0x50, 0x00, 0xfc, //
				0x00, 0x48, 0x00, 0xfe, //
				0x00, 0x44, 0x00, 0xff, //
				0x00, 0x42, 0x80, 0xff, //
				0x00, 0x41, 0xc0, 0xff, //
				0x80, 0x40, 0xe0, 0xff, //
				0x40, 0x40, 0xf0, 0xff, //
				0x20, 0x40, 0xf8, 0xff, //
				0x10, 0x40, 0xfc, 0xff, //
				0xf8, 0x40, 0xfc, 0xff, //
				0x80, 0x44, 0xfc, 0xff, //
				0x80, 0x4c, 0xe0, 0xff, //
				0x40, 0x52, 0xe0, 0xff, //
				0x40, 0x62, 0xf0, 0xff, //
				0x20, 0x41, 0xf0, 0xf7, //
				0x20, 0x01, 0xf8, 0xe3, //
				0x90, 0x00, 0xf8, 0x03, //
				0x90, 0x00, 0xf8, 0x01, //
				0x60, 0x00, 0xf8, 0x01, //
				0x00, 0x00, 0xf0, 0x00, //
		};

uint8_t textBuff[SCREEN_HEIGHT_PX / 8][SCREEN_WIDTH_PX / 8];
uint32_t chessBoardSpriteAddr[8][8];
Svga vga;

static void vga_irq_handler(void) {
	// Clear status bit to acknowledge interrupt
	AT91C_BASE_TC0->TC_SR;

	static int scanLine = 0;
	if (scanLine < SCREEN_HEIGHT_PX) {
		static uint8_t scanLineBufferA[SCREEN_WIDTH_PX / 8 + MOUSE_CURSOR_WIDTH_PX / 8];
		static uint8_t scanLineBufferB[SCREEN_WIDTH_PX / 8 + MOUSE_CURSOR_WIDTH_PX / 8];
		static uint8_t * scanLineBufferIndex = scanLineBufferA;

		//some jitter compensation
//		if (!(AT91C_BASE_TC0->TC_CV & 1))
//			AT91C_BASE_TC0->TC_CV;

		//start DMA transfer current line
		AT91C_BASE_SPI1->SPI_TPR = (uint32_t) scanLineBufferIndex;
		AT91C_BASE_SPI1->SPI_TCR = SCREEN_WIDTH_PX / 8;
		AT91C_BASE_SPI1->SPI_PTCR = AT91C_PDC_TXTEN;

		//prepare for next line
		scanLineBufferIndex = (scanLine & 1) ? scanLineBufferA : scanLineBufferB;
		int l = scanLine + 1;
		if (l >= SCREEN_HEIGHT_PX)
			l = 0;

		int buffOffset = 0;
#ifdef	TEXT_ENABLED
		int charRow = l & 7;
		int buffLine = l >> 3;
#endif	/*TEXT_ENABLED*/
#ifdef	CHESS_BOARD_ENABLED
#ifdef	TEXT_ENABLED
		/*display text 8x8*/
		for (int i = 0; i < vga.chessboard_Xpos; i++) {
			*(uint32_t*) (scanLineBufferIndex + buffOffset + 0) = font8x8win1251[textBuff[buffLine][buffOffset + 0] * 8 + charRow]
					+ (font8x8win1251[textBuff[buffLine][buffOffset + 1] * 8 + charRow] << 8)
					+ (font8x8win1251[textBuff[buffLine][buffOffset + 2] * 8 + charRow] << 16)
					+ (font8x8win1251[textBuff[buffLine][buffOffset + 3] * 8 + charRow] << 24);
			buffOffset += 4;
		}
#else	/*TEXT_ENABLED*/
		for (int i = 0; i < vga.chessboard_Xpos; i++) {
			*(uint32_t*) (scanLineBufferIndex + buffOffset + 0) = 0;
			buffOffset += 4;
		}
#endif	/*TEXT_ENABLED*/
		//chess board
		static int row = 0;
		if (l == vga.chessboard_Ypos)
			row = 0;
		if (row < 8) {
			static int pline = 0;
			for (int col = 0; col < 8; col++) {
				uint32_t x = chessBoardSpriteAddr[row][col];
				uint16_t pattern = (uint16_t) (((row ^ col) & 1) ? (0xEEEEEEEE >> (pline & 3)) : 0xFFFF);
				if (x) {
					int boffset = x + pline * CHESS48_BMP_WIDTH_BYTES;
					*(uint16_t*) (scanLineBufferIndex + buffOffset + 0) = *(uint16_t*) (boffset + 0) & pattern;
					*(uint16_t*) (scanLineBufferIndex + buffOffset + 2) = *(uint16_t*) (boffset + 2) & pattern;
					*(uint16_t*) (scanLineBufferIndex + buffOffset + 4) = *(uint16_t*) (boffset + 4) & pattern;
				} else {
					*(uint16_t*) (scanLineBufferIndex + buffOffset + 0) = pattern;
					*(uint16_t*) (scanLineBufferIndex + buffOffset + 2) = pattern;
					*(uint16_t*) (scanLineBufferIndex + buffOffset + 4) = pattern;
				}
				buffOffset += 6;
			}
			pline++;
			if (pline >= CHESS_PIECE_HEIGHT_PX) {
				pline = 0;
				row++;
			}
		}
#endif	/*CHESS_BOARD_ENABLED*/
#ifdef	TEXT_ENABLED
		/*display text 8x8*/
		while (buffOffset < (SCREEN_WIDTH_PX / 8)) {
			uint32_t x = font8x8win1251[textBuff[buffLine][buffOffset + 0] * 8 + charRow]
					+ (font8x8win1251[textBuff[buffLine][buffOffset + 1] * 8 + charRow] << 8)
					+ (font8x8win1251[textBuff[buffLine][buffOffset + 2] * 8 + charRow] << 16)
					+ (font8x8win1251[textBuff[buffLine][buffOffset + 3] * 8 + charRow] << 24);
			*(uint32_t*) (scanLineBufferIndex + buffOffset + 0) = x;
			buffOffset += 4;
		}
#else	/*TEXT_ENABLED*/
		for (int i = 0; i < vga.chessboard_Xpos; i++) {
			*(uint32_t*) (scanLineBufferIndex + buffOffset + 0) = 0;
			buffOffset += 4;
		}
#endif	/*TEXT_ENABLED*/

#ifdef	MOUSE_CURSOR_ENABLED
		//mouse cursor
		int mouseLine = l - vga.mouseYpos;
		if ((mouseLine >= 0) && (mouseLine < MOUSE_CURSOR_HEIGHT_PX)) {
			int byteOffset = vga.mouseXpos >> 3;
			int bitOffset = vga.mouseXpos - (byteOffset << 3);
			uint32_t a = scanLineBufferIndex[byteOffset + 2] + (scanLineBufferIndex[byteOffset + 1] << 8)
					+ (scanLineBufferIndex[byteOffset + 0] << 16);
			a &= ~(((uint32_t) (*(uint16_t*) (mouseCursor + mouseLine * 4 + 2)) << 8) >> bitOffset);
			a |= ((uint32_t) (*(uint16_t*) (mouseCursor + mouseLine * 4 + 0)) << 8) >> bitOffset;
			scanLineBufferIndex[byteOffset + 2] = (uint8_t) (a >> 0);
			scanLineBufferIndex[byteOffset + 1] = (uint8_t) (a >> 8);
			scanLineBufferIndex[byteOffset + 0] = (uint8_t) (a >> 16);
		}
#endif	/*MOUSE_CURSOR_ENABLED*/

	} else if (scanLine < SCREEN_HEIGHT_PX + 11) {
	} else if (scanLine == SCREEN_HEIGHT_PX + 11) {
		VGA_VSYNC_LOW;
	} else if (scanLine == SCREEN_HEIGHT_PX + 11 + 2) {
		VGA_VSYNC_HIGH;
	} else if (scanLine < SCREEN_HEIGHT_PX + 11 + 2 + 32) {

	} else {
		scanLine = -1;
	}
	scanLine++;
	VGA_DEBUG_HIGH;
	VGA_DEBUG_LOW;
}

static void vga_clock_init(void) {
	// Enable peripheral clock
	AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_TC0;

	TC_Configure(AT91C_BASE_TC0, 0 | AT91C_TC_CPCTRG | AT91C_TC_WAVE | AT91C_TC_WAVESEL_UP_AUTO | AT91C_TC_ACPA_SET | AT91C_TC_ACPC_CLEAR);
	AT91C_BASE_TC0->TC_RA = (BOARD_MCK * 10U / 2) / 31476U / 84U;
	AT91C_BASE_TC0->TC_RC = (BOARD_MCK / 2) / 31476U;

	// Configure and enable interrupt on RA compare
	IRQ_ConfigureIT(AT91C_ID_TC0, AT91C_AIC_PRIOR_HIGHEST, vga_irq_handler);
	AT91C_BASE_TC0->TC_IER = AT91C_TC_CPAS;
	IRQ_EnableIT(AT91C_ID_TC0);

	TC_Start(AT91C_BASE_TC0);
}

static void vga_spi_init(void) {
	SPI_Configure(AT91C_BASE_SPI1, AT91C_ID_SPI1,
	AT91C_SPI_MSTR | AT91C_SPI_MODFDIS);
	AT91C_BASE_SPI1->SPI_PTCR = AT91C_PDC_TXTDIS | AT91C_PDC_RXTDIS;
	AT91C_BASE_SPI1->SPI_TNPR = 0;
	AT91C_BASE_SPI1->SPI_TNCR = 0;
	AT91C_BASE_SPI1->SPI_RNPR = 0;
	AT91C_BASE_SPI1->SPI_RNCR = 0;
	AT91C_BASE_SPI1->SPI_TPR = 0;
	AT91C_BASE_SPI1->SPI_TCR = 0;
	AT91C_BASE_SPI1->SPI_RPR = 0;
	AT91C_BASE_SPI1->SPI_RCR = 0;

	SPI_ConfigureNPCS(AT91C_BASE_SPI1, 0, AT91C_SPI_BITS_8 | (SPI_CLOCK_DIV << 8));
	SPI_Enable(AT91C_BASE_SPI1);
}

void vga_init(void) {
	PIO_Configure(pinsVGA, PIO_LISTSIZE(pinsVGA));
	vga_clock_init();
	vga_spi_init();
#if defined(CHESS_BOARD_ENABLED) && defined(TEXT_ENABLED)
	for (int i = 0; i < 8; i++) {
		if (vga.chessboard_Ypos >= 8)
			textBuff[vga.chessboard_Ypos / 8 - 1][i * 6 + vga.chessboard_Xpos * 4 + 2] = 'A' + i;
		if (vga.chessboard_Ypos <= (SCREEN_HEIGHT_PX - CHESS_PIECE_HEIGHT_PX * 8 - 8))
			textBuff[6 * 8 + vga.chessboard_Ypos / 8][i * 6 + vga.chessboard_Xpos * 4 + 2] = 'A' + i;
		if (vga.chessboard_Xpos > 0)
			textBuff[6 * 8 + vga.chessboard_Ypos / 8 - 3 - i * 6][vga.chessboard_Xpos * 4 - 1] = '1' + i;
		if (vga.chessboard_Xpos < (SCREEN_WIDTH_PX - CHESS_PIECE_WIDTH_PX * 8) / 8 / 4)
			textBuff[6 * 8 + vga.chessboard_Ypos / 8 - 3 - i * 6][6 * 8 + vga.chessboard_Xpos * 4] = '1' + i;
	}
#endif	/*defined(CHESS_BOARD_ENABLED) && defined(TEXT_ENABLED)*/
}

void vga_chess_board_set(int row, int col, int p){
	chessBoardSpriteAddr[7 - row][col] = p ? chess48_map[p] + (uint32_t) &chess48_bmp[0] : 0;
}
