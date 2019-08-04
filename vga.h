/*
 * vga.h
 *
 *  Created on: 4 рту. 2019 у.
 *      Author: URA
 */

#ifndef VGA_H_
#define VGA_H_

#define	CHESS_BOARD_ENABLED
#define	MOUSE_CURSOR_ENABLED
#define	TEXT_ENABLED

#define	SCREEN_WIDTH_PX		640
#define	SCREEN_HEIGHT_PX	480

void vga_init(void);
void vga_chess_board_set(int row, int col, int p);
extern uint8_t textBuff[SCREEN_HEIGHT_PX / 8][SCREEN_WIDTH_PX / 8];

typedef struct {
#ifdef	CHESS_BOARD_ENABLED
	uint16_t chessboard_Xpos;
	uint16_t chessboard_Ypos;
#endif	/*CHESS_BOARD_ENABLED*/
#ifdef	MOUSE_CURSOR_ENABLED
	uint16_t mouseXpos;
	uint16_t mouseYpos;
#endif	/*MOUSE_CURSOR_ENABLED*/
} Svga;

extern Svga vga;

#endif /* VGA_H_ */
