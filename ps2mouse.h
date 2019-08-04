/*
 * ps2mouse.h
 *
 *  Created on: 3 рту. 2019 у.
 *      Author: URA
 */

#ifndef PS2MOUSE_H_
#define PS2MOUSE_H_
#include "common.h"
extern void ps2Init(void);
extern void ps2Write(uint8_t byte);
extern uint8_t ps2Read(void);
extern void ps2DelayUs(int us);

#endif /* PS2MOUSE_H_ */
