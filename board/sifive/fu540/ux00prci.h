/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2019 SiFive, Inc
 */

#ifndef _SIFIVE_UX00PRCI_H
#define _SIFIVE_UX00PRCI_H

/* Register offsets */
#define UX00PRCI_HFROSCCFG          (0x0000)
#define UX00PRCI_COREPLLCFG         (0x0004)
#define UX00PRCI_COREPLLOUT         (0x0008)
#define UX00PRCI_DDRPLLCFG          (0x000C)
#define UX00PRCI_DDRPLLOUT          (0x0010)
#define UX00PRCI_GEMGXLPLLCFG       (0x001C)
#define UX00PRCI_GEMGXLPLLOUT       (0x0020)
#define UX00PRCI_CORECLKSELREG      (0x0024)
#define UX00PRCI_DEVICESRESETREG    (0x0028)
#define UX00PRCI_CLKMUXSTATUSREG    (0x002C)
#define UX00PRCI_PROCMONCFG         (0x00F0)

#define PLL_R(x)              (((x) & 0x3F) << 0)
#define PLL_F(x)              (((x) & 0x1FF) << 6)
#define PLL_Q(x)              (((x) & 0x7) << 15)
#define PLL_RANGE(x)          (((x) & 0x7) << 18)
#define PLL_BYPASS(x)         (((x) & 0x1) << 24)
#define PLL_FSE(x)            (((x) & 0x1) << 25)
#define PLL_LOCK(x)           (((x) & 0x1) << 31)

#define PLLOUT_DIV(x)         (((x) & 0x7F) << 0)
#define PLLOUT_DIV_BY_1(x)    (((x) & 0x1) << 8)
#define PLLOUT_CLK_EN(x)      (((x) & 0x1) << 31)

#define PLL_R_default              0x1
#define PLL_F_default              0x1F
#define PLL_Q_default              0x3
#define PLL_RANGE_default          0x0
#define PLL_BYPASS_default         0x1
#define PLL_FSE_default            0x1

#define PLLOUT_DIV_default         0x0
#define PLLOUT_DIV_BY_1_default    0x0
#define PLLOUT_CLK_EN_default      0x0

#define PLL_CORECLKSEL_HFXIN       0x1
#define PLL_CORECLKSEL_COREPLL     0x0

#define DEVICESRESET_DDR_CTRL_RST_N(x)    (((x) & 0x1) << 0)
#define DEVICESRESET_DDR_AXI_RST_N(x)     (((x) & 0x1) << 1)
#define DEVICESRESET_DDR_AHB_RST_N(x)     (((x) & 0x1) << 2)
#define DEVICESRESET_DDR_PHY_RST_N(x)     (((x) & 0x1) << 3)
#define DEVICESRESET_GEMGXL_RST_N(x)      (((x) & 0x1) << 5)

#define CLKMUX_STATUS_TLCLKSEL         (0x1 << 1)

#endif // _SIFIVE_UX00PRCI_H
