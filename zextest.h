/* zextest.h
 * Header for zextest example.
 *
 * Copyright (c) 2012, 2016 Lin Ke-Fong
 *
 * This code is free, do whatever you want with it.
 */

#ifndef __ZEXTEST_INCLUDED__
#define __ZEXTEST_INCLUDED__

#define MEMSIZE (1 << 16)

#include "z80emu.h"

typedef struct ZEXTEST {

	Z80_STATE	state;
	unsigned char	memory[MEMSIZE];

} ZEXTEST;

#endif
