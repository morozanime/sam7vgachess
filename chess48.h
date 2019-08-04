/*
 * chess48.h
 *
 *  Created on: 31 èþë. 2019 ã.
 *      Author: URA
 */

#ifndef CHESS48_H_
#define CHESS48_H_

#define	CHESS_PIECE_WIDTH_PX	48
#define	CHESS_PIECE_WIDTH_BYTES	6
#define	CHESS_PIECE_HEIGHT_PX	48

#define	CHESS48_BMP_WIDTH_PX	288
#define	CHESS48_BMP_WIDTH_BYTES	(288/8)
#include "common.h"
extern const uint8_t chess48_bmp[];
extern const int chess48_map[16];

#endif /* CHESS48_H_ */
