// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 SiFive, Inc
 *
 * Authors:
 *   Pragnesh Patel <pragnesh.patel@sifive.com>
 */

#include <common.h>
#include <spl.h>
#include <misc.h>
#include <dm.h>

#include "fu540-memory-map.h"

#define DDRCTLPLL_F 55
#define DDRCTLPLL_Q 2

#define PHY_NRESET 0x1000

long nsec_per_cyc = 300; /* 33.333MHz */
void nsleep(long nsec)
{
	long step = nsec_per_cyc * 2;

	while (nsec > 0)
		nsec -= step;
}

void init_clk_and_ddr(void)
{
	int ret;
	struct udevice *dev;

	/* PRCI init */
	ret = uclass_get_device(UCLASS_CLK, 0, &dev);
	if (ret) {
		debug("Clock init failed: %d\n", ret);
		return;
	}

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		printf("DRAM init failed: %d\n", ret);
		return;
	}

	/*
	 * GEMGXL init VSC8541 PHY reset sequence;
	 * leave pull-down active for 2ms
	 */
	nsleep(2000000);
	/* Set GPIO 12 (PHY NRESET) to OE=1 and OVAL=1 */
	GPIO_REG(GPIO_OUTPUT_VAL) |= PHY_NRESET;
	GPIO_REG(GPIO_OUTPUT_EN) |= PHY_NRESET;
	nsleep(100);

	/* Reset PHY again to enter unmanaged mode */
	GPIO_REG(GPIO_OUTPUT_VAL) &= ~PHY_NRESET;
	nsleep(100);
	GPIO_REG(GPIO_OUTPUT_VAL) |= PHY_NRESET;
	nsleep(15000000);
}

void board_init_f(ulong dummy)
{
	int ret;

	ret = spl_early_init();
	if (ret)
		panic("spl_early_init() failed: %d\n", ret);

	arch_cpu_init_dm();

	init_clk_and_ddr();

	preloader_console_init();
}
