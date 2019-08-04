//------------------------------------------------------------------------------
#include 	"main.h"
#include 	"common.h"
#include    "chess.h"
#include 	"ps2mouse.h"
#include 	"vga.h"

void scrollUp(void) {
	for (int i = 0; i < 480 / 8 - 1; i++)
		for (int j = 0; j < 640 / 8; j++)
			textBuff[i][j] = textBuff[i + 1][j];
	memset((void*) (&textBuff[480 / 8 - 1][0]), 0, 640 / 8);
}

void putChar(char c) {
	static int line = 0;
	static int pos = 0;
	if (c == 0x0d)
		pos = 0;
	else if (c == 0x0a) {
		pos = 0;
		if (line >= 480 / 8 - 1)
			scrollUp();
		else
			line++;
	} else {
		if (pos >= 640 / 8 - 1) {
			pos = 0;
			if (line >= 480 / 8 - 1)
				scrollUp();
			else
				line++;
			textBuff[line][pos] = c;
		} else
			textBuff[line][pos++] = c;
	}
}

void putString(char * str) {
	while (*str)
		putChar(*(str++));
}

int main(void) {
// DBGU output configuration
	TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
	printf("-- Getting Started Project %s --\n\r", SOFTPACK_VERSION);
	printf("-- %s\n\r", BOARD_NAME);
	printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);

	vga_init();
// Main loop

	ps2Init();
	ps2Write(0xff);
	ps2Read();
	ps2Read();
	ps2Read();
	ps2Write(0xf0);
	ps2Read();
	ps2DelayUs(100);

	chess_init();
	while (1) {

//		printf("A=%08X\n\r", AT91C_BASE_PIOA->PIO_PDSR);
	}
}

int getMove_mouse(void) {
	int dx, dy;
	int result = 0;
	ps2Write(0xeb);
	ps2Read();
	uint8_t a;
	a = ps2Read();
	dx = (int8_t) ps2Read();
	dy = (int8_t) ps2Read();
	int x = vga.mouseXpos + dx;
	int y = vga.mouseYpos - dy;
	if (x < 0)
		x = 0;
	else if (x > 640 - 3)
		x = 640 - 3;
	vga.mouseXpos = (uint16_t) x;
	if (y < 0)
		y = 0;
	else if (y > 480 - 3)
		y = 480 - 3;
	vga.mouseYpos = (uint16_t) y;
	static uint8_t btns = 0;
	if (a & (a ^ btns) & 1) {
		if (vga.mouseXpos >= vga.chessboard_Xpos * 32
				&& vga.mouseXpos < (vga.chessboard_Xpos * 32 + 6 * 8 * 8)
				&& vga.mouseYpos >= vga.chessboard_Ypos
				&& vga.mouseYpos < (vga.chessboard_Ypos + 6 * 8 * 8)) {
			result = (((vga.mouseXpos - vga.chessboard_Xpos * 32) / (6 * 8) + 'a') << 8)
					+ (-(vga.mouseYpos - vga.chessboard_Ypos) / (6 * 8) + '8');
		}
	} else if (a & (a ^ btns) & 2) {
		result = -1;
	}
	btns = a;
	return result;
}

void panel_init(void) {
}

VOID sound_yourmove() {

}

BOOL panel_getmove(LOC from, LOC to) {
	char buff[4];
	int a = getMove_mouse();
	if (a > 0) {
		buff[0] = (char) (a >> 8);
		buff[1] = (char) a;
	} else
		return FALSE;
	putChar(buff[0]);
	putChar(buff[1]);
	do {
		a = getMove_mouse();
		if(a < 0){
			putChar(';');
			return FALSE;
		}
		if (a > 0) {
			buff[2] = (char) (a >> 8);
			buff[3] = (char) a;
			break;
		}
	} while (a == 0);
	putChar('-');
	putChar(buff[2]);
	putChar(buff[3]);
	putChar(',');
//
//	buff[0] = DBGU_GetChar();
//	printf("%c", buff[0]);
//	putChar(buff[0]);
//	buff[1] = DBGU_GetChar();
//	printf("%c", buff[1]);
//	putChar(buff[1]);
//	buff[2] = DBGU_GetChar();
//	printf("%c", buff[2]);
//	putChar(buff[2]);
//	buff[3] = DBGU_GetChar();
//	printf("%c", buff[3]);
//	putChar(buff[3]);

	from[1] = buff[0] - 'a';
	from[0] = buff[1] - '1';
	to[1] = buff[2] - 'a';
	to[0] = buff[3] - '1';
	return TRUE;
}

VOID sound_illegal() {
}

VOID panel_draw(COORD row, COORD col, PIECE p) {
	vga_chess_board_set(row, col, p);
}

VOID sound_capture() {
//	printf("sound_capture\n");
}

VOID panel_cls(VOID) {
}
