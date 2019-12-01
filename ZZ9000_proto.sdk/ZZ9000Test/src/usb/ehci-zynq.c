// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014, Xilinx, Inc
 *
 * USB Low level initialization(Specific to zynq)
 */

#include "usb.h"
#include "ehci-ci.h"
#include "ulpi.h"
#include "ehci.h"
#include <stdio.h>
#include <xusbps_hw.h>

#define EINVAL 1

// see zynq TRM
#define USB_BASE_ADDR 0xE0002000

int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	enum usb_init_type type;

	if (index > 0) {
		printf("[ehci_hcd_init] index out of range: %d\n",index);
		return -EINVAL;
	}

	struct usb_ehci *ehci = (struct usb_ehci *)(USB_BASE_ADDR);

	if (hccr && hcor) {
		*hccr = (struct ehci_hccr *)((uint32_t)&ehci->caplength);
		*hcor = (struct ehci_hcor *)((uint32_t)*hccr + HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));
	}

	return 0;
}

int ehci_zynq_probe(struct zynq_ehci_priv *priv)
{
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
	struct ulpi_viewport ulpi_vp;
	/* Used for writing the ULPI data address */
	struct ulpi_regs *ulpi = (struct ulpi_regs *)0;
	int ret;

	struct usb_ehci *ehci = (struct usb_ehci *)(USB_BASE_ADDR);
	priv->ehci = ehci;

	XUsbPs_ResetHw(USB_BASE_ADDR);

	hccr = (struct ehci_hccr *)((uint32_t)&priv->ehci->caplength);
	hcor = (struct ehci_hcor *)((uint32_t) hccr + HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	printf("[ehci-zynq] hccr: %p hcor: %p\n", hccr, hcor);

	priv->ehcictrl.hccr = hccr;
	priv->ehcictrl.hcor = hcor;

	ulpi_vp.viewport_addr = (u32)&priv->ehci->ulpi_viewpoint;
	ulpi_vp.port_num = 0;

	printf("[ehci-zynq] viewport_addr: %p\n", &priv->ehci->ulpi_viewpoint);

	ret = ulpi_init(&ulpi_vp);
	if (ret) {
		puts("zynq ULPI viewport init failed\n");
		return -1;
	}

	/* ULPI set flags */

	// dp and dm pulldown = host mode
	// extvbusind = vbus indicator input
	ulpi_write(&ulpi_vp, &ulpi->otg_ctrl,
		   ULPI_OTG_DP_PULLDOWN | ULPI_OTG_DM_PULLDOWN |
		   ULPI_OTG_EXTVBUSIND);
	ulpi_write(&ulpi_vp, &ulpi->function_ctrl,
		   ULPI_FC_FULL_SPEED | ULPI_FC_OPMODE_NORMAL |
		   ULPI_FC_SUSPENDM);
	ulpi_write(&ulpi_vp, &ulpi->iface_ctrl, 0);

	/* drive external vbus switch */
	ulpi_write(&ulpi_vp, &ulpi->otg_ctrl_set,
		   ULPI_OTG_DRVVBUS | ULPI_OTG_DRVVBUS_EXT);

	return ehci_register(&priv->ehcictrl, hccr, hcor, NULL, 0, USB_INIT_HOST);
}
