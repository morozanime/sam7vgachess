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
#include <utility/trace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(cortexm3)
#include <systick/systick.h>
#endif

#endif /* MAIN_H_ */
