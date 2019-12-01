/*
 * MNT ZZ9000 Amiga Graphics and Coprocessor Card Operating System (ZZ9000OS)
 *
 * Copyright (C) 2019, Lukas F. Hartmann <lukas@mntre.com>
 *                     MNT Research GmbH, Berlin
 *                     https://mntre.com
 *
 * More Info: https://mntre.com/zz9000
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * GNU General Public License v3.0 or later
 *
 * https://spdx.org/licenses/GPL-3.0-or-later.html
 *
 * Some portions (from example code, weird custom license) Copyright (C) 2010 - 2015 Xilinx, Inc.
 *
*/

#include <stdio.h>
#include "platform.h"
#include <xil_printf.h>
#include <xil_cache.h>
#include <xil_mmu.h>
#include "sleep.h"
#include "xparameters.h"
#include <xscugic.h>
#include "usb/ehci.h"

struct zynq_ehci_priv _zynq_ehci;

/*
 * https://www.cypress.com/file/134171/download
 * https://elixir.bootlin.com/u-boot/latest/source/common/usb_storage.c#L653
 *
 * "Enumeration directly follows the device detection, and is the process
of assigning a unique address to a newly attached device. Configuration is the process of determining a device‘s
capabilities by an exchange of device requests. The requests that the host uses to learn about a device are called
standard requests and must support these requests on all USB devices."
 */

void test_usb() {
	printf("[USB] trying to probe zynq ehci...\n");
	ehci_zynq_probe(&_zynq_ehci);
	printf("[USB] probed!\n");
	usb_init();
	printf("[USB] initialized!\n");
}
