#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"

#include "xemacps.h"

static XEmacPs* EmacPsInstancePtr;

int init_ethernet() {

	XEmacPs_Config *Config;
	XEmacPs_Bd BdTemplate;
	long Status;

	Config = XEmacPs_LookupConfig(XPAR_XEMACPS_0_DEVICE_ID);
	Status = XEmacPs_CfgInitialize(EmacPsInstancePtr, Config, Config->BaseAddress);

	if (Status != XST_SUCCESS) {
		printf("EMAC: Error in initialize");
		return XST_FAILURE;
	}

	//XEmacPsClkSetup(EmacPsInstancePtr, EmacPsIntrId);
}
