/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2019 SiFive, Inc
 */

#ifndef FU540_CACHE_H
#define FU540_CACHE_H

#define CACHE_CTRL_ADDR               _AC(0x2010000, UL)

void cache_enable_ways(u64 base_addr, u8 value);

#endif /* FU540_CACHE_H */
