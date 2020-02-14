/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright (C) 2020, SiFive, Inc
 *
 * Authors:
 *   Pragnesh Patel <pragnesh.patel@sifive.com>
 */

#define DENALI_CTL_0	0
#define DENALI_CTL_21	21
#define DENALI_CTL_120	120
#define DENALI_CTL_132	132
#define DENALI_CTL_136	136
#define DENALI_CTL_170	170
#define DENALI_CTL_181	181
#define DENALI_CTL_182	182
#define DENALI_CTL_184	184
#define DENALI_CTL_208	208
#define DENALI_CTL_209	209
#define DENALI_CTL_210	210
#define DENALI_CTL_212	212
#define DENALI_CTL_214	214
#define DENALI_CTL_216	216
#define DENALI_CTL_224	224
#define DENALI_CTL_225	225
#define DENALI_CTL_260	260

#define DENALI_PHY_1152	1152
#define DENALI_PHY_1214	1214

#define PAYLOAD_DEST	0x80000000
#define DDR_MEM_SIZE	(8UL * 1024UL * 1024UL * 1024UL)

#define DRAM_CLASS_OFFSET                   8
#define DRAM_CLASS_DDR4                     0xA
#define OPTIMAL_RMODW_EN_OFFSET             0
#define DISABLE_RD_INTERLEAVE_OFFSET        16
#define OUT_OF_RANGE_OFFSET                 1
#define MULTIPLE_OUT_OF_RANGE_OFFSET        2
#define PORT_COMMAND_CHANNEL_ERROR_OFFSET   7
#define MC_INIT_COMPLETE_OFFSET             8
#define LEVELING_OPERATION_COMPLETED_OFFSET 22
#define DFI_PHY_WRLELV_MODE_OFFSET          24
#define DFI_PHY_RDLVL_MODE_OFFSET           24
#define DFI_PHY_RDLVL_GATE_MODE_OFFSET      0
#define VREF_EN_OFFSET                      24
#define PORT_ADDR_PROTECTION_EN_OFFSET      0
#define AXI0_ADDRESS_RANGE_ENABLE           8
#define AXI0_RANGE_PROT_BITS_0_OFFSET       24
#define RDLVL_EN_OFFSET                     16
#define RDLVL_GATE_EN_OFFSET                24
#define WRLVL_EN_OFFSET                     0

#define PHY_RX_CAL_DQ0_0_OFFSET             0
#define PHY_RX_CAL_DQ1_0_OFFSET             16

struct fu540_ddrctl {
	volatile u32 denali_ctl[265];
};

struct fu540_ddrphy {
	volatile u32 denali_phy[1215];
};

struct fu540_sdram_params {
	struct fu540_ddrctl pctl_regs;
	struct fu540_ddrphy phy_regs;
};

struct sifive_dmc_plat {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_sifive_fu540_dmc dtplat;
#else
	struct fu540_sdram_params sdram_params;
#endif
};

/**
 * struct ddr_info
 *
 * @dev				: pointer for the device
 * @info			: UCLASS RAM information
 * @ctl				: DDR controleur base address
 * @phy				: DDR PHY base address
 * @ctrl			: DDR control base address
 * @physical_filter_ctrl	: DDR physical filter control base address
 */
struct ddr_info {
	struct udevice *dev;
	struct ram_info info;
	struct fu540_ddrctl *ctl;
	struct fu540_ddrphy *phy;
	u32 *physical_filter_ctrl;
};
