// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * (C) Copyright 2020 SiFive, Inc.
 *
 * Authors:
 *   Pragnesh Patel <pragnesh.patel@sifive.com>
 */

#include <common.h>
#include <dm.h>
#include <init.h>
#include <ram.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include "sdram_fu540.h"

/* n: Unit bytes */
void sdram_copy_to_reg(volatile u32 *dest, volatile u32 *src, u32 n)
{
	int i;

	for (i = 0; i < n / sizeof(u32); i++) {
		writel(*src, dest);
		src++;
		dest++;
	}
}

static void ddr_setuprangeprotection(volatile u32 *ctl, u64 end_addr)
{
	writel(0x0, DENALI_CTL_209 + ctl);
	u32 end_addr_16kblocks = ((end_addr >> 14) & 0x7FFFFF) - 1;

	writel(end_addr_16kblocks, DENALI_CTL_210 + ctl);
	writel(0x0, DENALI_CTL_212 + ctl);
	writel(0x0, DENALI_CTL_214 + ctl);
	writel(0x0, DENALI_CTL_216 + ctl);
	setbits_le32(DENALI_CTL_224 + ctl,
		     0x3 << AXI0_RANGE_PROT_BITS_0_OFFSET);
	writel(0xFFFFFFFF, DENALI_CTL_225 + ctl);
	setbits_le32(DENALI_CTL_208 + ctl, 0x1 << AXI0_ADDRESS_RANGE_ENABLE);
	setbits_le32(DENALI_CTL_208 + ctl,
		     0x1 << PORT_ADDR_PROTECTION_EN_OFFSET);
}

static void ddr_start(volatile u32 *ctl, u32 *physical_filter_ctrl, u64 ddr_end)
{
	setbits_le32(DENALI_CTL_0 + ctl, 0x1);
	while ((readl(DENALI_CTL_132 + ctl) & (1 << MC_INIT_COMPLETE_OFFSET))
	       == 0) {
	}

	// Disable the BusBlocker in front of the controller AXI slave ports
	volatile u64 *filterreg = (volatile u64 *)physical_filter_ctrl;

	filterreg[0] = 0x0f00000000000000UL | (ddr_end >> 2);
}

static u64 ddr_phy_fixup(volatile u32 *ddrphyreg)
{
	// return bitmask of failed lanes

	u64 fails     = 0;
	u32 slicebase = 0;
	u32 dq        = 0;

	// check errata condition
	for (u32 slice = 0; slice < 8; slice++) {
		u32 regbase = slicebase + 34;

		for (u32 reg = 0; reg < 4; reg++) {
			u32 updownreg = readl(regbase + reg + ddrphyreg);

			for (u32 bit = 0; bit < 2; bit++) {
				u32 phy_rx_cal_dqn_0_offset;

				if (bit == 0) {
					phy_rx_cal_dqn_0_offset =
						PHY_RX_CAL_DQ0_0_OFFSET;
				} else {
					phy_rx_cal_dqn_0_offset =
						PHY_RX_CAL_DQ1_0_OFFSET;
				}

				u32 down = (updownreg >>
					    phy_rx_cal_dqn_0_offset) & 0x3F;
				u32 up   = (updownreg >>
					    (phy_rx_cal_dqn_0_offset + 6)) &
					    0x3F;

				u8 failc0 = ((down == 0) && (up == 0x3F));
				u8 failc1 = ((up == 0) && (down == 0x3F));

				// print error message on failure
				if (failc0 || failc1) {
					if (fails == 0)
						printf("DDR error in fixing up\n");

					fails |= (1 << dq);

					char slicelsc = '0';
					char slicemsc = '0';

					slicelsc += (dq % 10);
					slicemsc += (dq / 10);
					printf("S ");
					printf("%c", slicemsc);
					printf("%c", slicelsc);

					if (failc0)
						printf("U");
					else
						printf("D");

					printf("\n");
				}
				dq++;
			}
		}
		slicebase += 128;
	}
	return(0);
}

static u32 ddr_getdramclass(volatile u32 *ctl)
{
	u32 reg = readl(DENALI_CTL_0 + ctl);

	return ((reg >> DRAM_CLASS_OFFSET) & 0xF);
}

static __maybe_unused int fu540_ddr_setup(struct udevice *dev)
{
	struct ddr_info *priv = dev_get_priv(dev);
	struct sifive_dmc_plat *plat = dev_get_platdata(dev);

	int ret, i;
	u32 physet;

	volatile u32 *denali_ctl =  &priv->ctl->denali_ctl[0];
	volatile u32 *denali_phy =  &priv->phy->denali_phy[0];

	ret = dev_read_u32_array(dev, "sifive,sdram-params",
				 (u32 *)&plat->sdram_params,
				 sizeof(plat->sdram_params) / sizeof(u32));
	if (ret) {
		printf("%s: Cannot read sifive,sdram-params %d\n",
		       __func__, ret);
		return ret;
	}

	struct fu540_sdram_params *params = &plat->sdram_params;

	sdram_copy_to_reg(&priv->ctl->denali_ctl[0],
			  &params->pctl_regs.denali_ctl[0],
			  sizeof(struct fu540_ddrctl));

	/* phy reset */
	for (i = DENALI_PHY_1152; i <= DENALI_PHY_1214; i++) {
		physet = params->phy_regs.denali_phy[i];
		priv->phy->denali_phy[i] = physet;
	}

	for (i = 0; i < DENALI_PHY_1152; i++) {
		physet = params->phy_regs.denali_phy[i];
		priv->phy->denali_phy[i] = physet;
	}

	/* Disable read interleave DENALI_CTL_120 */
	setbits_le32(DENALI_CTL_120 + denali_ctl,
		     1 << DISABLE_RD_INTERLEAVE_OFFSET);

	/* Disable optimal read/modify/write logic DENALI_CTL_21 */
	clrbits_le32(DENALI_CTL_21 + denali_ctl, 1 << OPTIMAL_RMODW_EN_OFFSET);

	/* Enable write Leveling DENALI_CTL_170 */
	setbits_le32(DENALI_CTL_170 + denali_ctl, (1 << WRLVL_EN_OFFSET)
		     | (1 << DFI_PHY_WRLELV_MODE_OFFSET));

	/* Enable read leveling DENALI_CTL_181 and DENALI_CTL_260 */
	setbits_le32(DENALI_CTL_181 + denali_ctl,
		     1 << DFI_PHY_RDLVL_MODE_OFFSET);
	setbits_le32(DENALI_CTL_260 + denali_ctl, 1 << RDLVL_EN_OFFSET);

	/* Enable read leveling gate DENALI_CTL_260 and DENALI_CTL_182 */
	setbits_le32(DENALI_CTL_260 + denali_ctl, 1 << RDLVL_GATE_EN_OFFSET);
	setbits_le32(DENALI_CTL_182 + denali_ctl,
		     1 << DFI_PHY_RDLVL_GATE_MODE_OFFSET);

	if (ddr_getdramclass(denali_ctl) == DRAM_CLASS_DDR4) {
		/* Enable vref training DENALI_CTL_184 */
		setbits_le32(DENALI_CTL_184 + denali_ctl, 1 << VREF_EN_OFFSET);
	}

	/* Mask off leveling completion interrupt DENALI_CTL_136 */
	setbits_le32(DENALI_CTL_136 + denali_ctl,
		     1 << LEVELING_OPERATION_COMPLETED_OFFSET);

	/* Mask off MC init complete interrupt DENALI_CTL_136 */
	setbits_le32(DENALI_CTL_136 + denali_ctl, 1 << MC_INIT_COMPLETE_OFFSET);

	/* Mask off out of range interrupts DENALI_CTL_136 */
	setbits_le32(DENALI_CTL_136 + denali_ctl, (1 << OUT_OF_RANGE_OFFSET)
		     | (1 << MULTIPLE_OUT_OF_RANGE_OFFSET));

	/* set up range protection */
	ddr_setuprangeprotection(denali_ctl, DDR_MEM_SIZE);

	/* Mask off port command error interrupt DENALI_CTL_136 */
	setbits_le32(DENALI_CTL_136 + denali_ctl,
		     1 << PORT_COMMAND_CHANNEL_ERROR_OFFSET);

	const u64 ddr_size = DDR_MEM_SIZE;
	const u64 ddr_end = PAYLOAD_DEST + ddr_size;

	ddr_start(denali_ctl, priv->physical_filter_ctrl, ddr_end);

	ddr_phy_fixup(denali_phy);

	/* check size */
	priv->info.size = get_ram_size((long *)priv->info.base,
				       DDR_MEM_SIZE);

	printf("priv->info.size = %lx\n", priv->info.size);
	debug("%s : %lx\n", __func__, priv->info.size);

	/* check memory access for all memory */

	if (priv->info.size != DDR_MEM_SIZE) {
		printf("DDR invalid size : 0x%lx, expected 0x%lx\n",
		       priv->info.size, DDR_MEM_SIZE);
		return -EINVAL;
	}

	return 0;
}

static int fu540_ddr_probe(struct udevice *dev)
{
	struct ddr_info *priv = dev_get_priv(dev);

#if defined(CONFIG_SPL_BUILD)
	struct regmap *map;
	int ret;

	debug("FU540 DDR probe\n");
	priv->dev = dev;

	ret = regmap_init_mem(dev_ofnode(dev), &map);
	if (ret)
		return ret;

	priv->ctl = regmap_get_range(map, 0);
	priv->phy = regmap_get_range(map, 1);
	priv->physical_filter_ctrl = regmap_get_range(map, 2);

	priv->info.base = CONFIG_SYS_SDRAM_BASE;

	priv->info.size = 0;
	return fu540_ddr_setup(dev);
#else
	priv->info.base = CONFIG_SYS_SDRAM_BASE;
	priv->info.size = DDR_MEM_SIZE;
#endif
	return 0;
}

static int fu540_ddr_get_info(struct udevice *dev, struct ram_info *info)
{
	struct ddr_info *priv = dev_get_priv(dev);

	*info = priv->info;

	return 0;
}

static struct ram_ops fu540_ddr_ops = {
	.get_info = fu540_ddr_get_info,
};

static const struct udevice_id fu540_ddr_ids[] = {
	{ .compatible = "sifive,fu540-ddr" },
	{ }
};

U_BOOT_DRIVER(fu540_ddr) = {
	.name = "fu540_ddr",
	.id = UCLASS_RAM,
	.of_match = fu540_ddr_ids,
	.ops = &fu540_ddr_ops,
	.probe = fu540_ddr_probe,
	.priv_auto_alloc_size = sizeof(struct ddr_info),
	.platdata_auto_alloc_size = sizeof(struct sifive_dmc_plat),
};
