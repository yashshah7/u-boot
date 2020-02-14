// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 SiFive, Inc
 */
#include <asm/io.h>

/* Register offsets */
#define CACHE_ENABLE           0x008

/* Enable ways; allow cache to use these ways */
void cache_enable_ways(u64 base_addr, u8 value)
{
	volatile u32 *enable = (volatile u32 *)(base_addr +
					  CACHE_ENABLE);
	/* memory barrier */
	mb();
	(*enable) = value;
	/* memory barrier */
	mb();
}
