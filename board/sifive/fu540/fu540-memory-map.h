/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2019 SiFive, Inc
 */

#ifndef FU540_MEMORY_MAP
#define FU540_MEMORY_MAP

#include <asm/arch/gpio.h>
#include "ux00prci.h"

/****************************************************************************
 * Platform definitions
 *****************************************************************************/

/* Memory map */
#define GPIO_CTRL_ADDR                 _AC(0x10060000, UL)

#define PHYSICAL_FILTER_CTRL_ADDR      _AC(0x100b8000, UL)

#define UX00DDR_CTRL_ADDR              _AC(0x100b0000, UL)
#define UX00PRCI_CTRL_ADDR             _AC(0x10000000, UL)

/* Helper functions */
#define _REG32(p, i)    (*(volatile uint32_t *)((p) + (i)))

#define UX00PRCI_REG(offset)  \
	_REG32(UX00PRCI_CTRL_ADDR, \
			offset)

#define GPIO_REG(offset)      _REG32(GPIO_CTRL_ADDR, offset)

#endif /* FU540_MEMORY_MAP */
