#define ZZ_ENET

#ifdef ZZ_ENET

#include <stdio.h>
#include "platform.h"
#include <xil_printf.h>
#include <xil_cache.h>
#include <xil_mmu.h>
#include "sleep.h"
#include "xparameters.h"
#include <xemacps.h>
#include <xscugic.h>
#include "ethernet.h"

static XEmacPs EmacPsInstance;
static XScuGic IntcInstance;

// could also be 55, 77 (eth1), see interrupts.pdf last page
// XPS_GEM0_INT_ID == 54
#define EMACPS_IRPT_INTR	XPS_GEM0_INT_ID


#define SLCR_LOCK_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x4)
#define SLCR_UNLOCK_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x8)
#define SLCR_GEM0_CLK_CTRL_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x140)
#define SLCR_GEM1_CLK_CTRL_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x144)


#define SLCR_LOCK_KEY_VALUE		0x767B
#define SLCR_UNLOCK_KEY_VALUE		0xDF0D
#define SLCR_ADDR_GEM_RST_CTRL		(XPS_SYS_CTRL_BASEADDR + 0x214)

#define EMACPS_LOOPBACK_SPEED    100
#define EMACPS_LOOPBACK_SPEED_1G 1000
#define EMACPS_PHY_DELAY_SEC     4	//Amount of time to delay waiting on PHY to reset
#define EMACPS_SLCR_DIV_MASK	0xFC0FC0FF

static s32 GemVersion;
uint8_t EmacPsMAC[6] = {0x68,0x82,0xf2,0x00,0x01,0x00};

static volatile s32 DeviceErrors = 0;
static volatile u32 FramesTx = 0;
static volatile u32 FramesRx = 0;
static u32 TxFrameLength;

typedef char EthernetFrame[XEMACPS_MAX_VLAN_FRAME_SIZE_JUMBO] __attribute__ ((aligned(64)));

char* TxFrame = (char*)TX_FRAME_ADDRESS;		/* Transmit buffer */
char* RxFrame = (char*)RX_FRAME_ADDRESS;		/* Receive buffer */

/*
 * Buffer descriptors are allocated in uncached memory. The memory is made
 * uncached by setting the attributes appropriately in the MMU table.
 */
#define RXBD_SPACE_BYTES XEmacPs_BdRingMemCalc(XEMACPS_BD_ALIGNMENT, RXBD_CNT)
#define TXBD_SPACE_BYTES XEmacPs_BdRingMemCalc(XEMACPS_BD_ALIGNMENT, TXBD_CNT)


#define FIRST_FRAGMENT_SIZE 64

#define PHY_DETECT_REG1 2
#define PHY_DETECT_REG2 3
#define PHY_ID_MARVELL	0x141
#define PHY_ID_MICREL_KSZ9031 0x22

static void XEmacPsSendHandler(void *Callback);
static void XEmacPsRecvHandler(void *Callback);
static void XEmacPsErrorHandler(void *Callback, u8 direction, u32 word);
LONG EmacPsUtilFrameVerify(EthernetFrame * CheckFrame, EthernetFrame * ActualFrame);
void EmacPsUtilFrameHdrFormatType(EthernetFrame * FramePtr, u16 FrameType);
void EmacPsUtilFrameHdrFormatMAC(EthernetFrame * FramePtr, char *DestAddr);
void EmacPsUtilFrameSetPayloadData(EthernetFrame * FramePtr, u32 PayloadSize);
void EmacPsUtilFrameMemClear(EthernetFrame * FramePtr);
LONG setup_phy(XEmacPs * EmacPsInstancePtr, u32 Speed);
LONG EmacPsUtilMarvellPhyLoopback(XEmacPs * EmacPsInstancePtr, u32 Speed, u32 PhyAddr);
static LONG EmacPsSetupIntrSystem(XScuGic *IntcInstancePtr, XEmacPs *EmacPsInstancePtr, u16 EmacPsIntrId);

static u32 micrel_auto_negotiate(XEmacPs *xemacpsp, u32 phy_addr);

void XEmacPsClkSetup(XEmacPs *EmacPsInstancePtr, u16 EmacPsIntrId)
{
	u32 SlcrTxClkCntrl;
	//u32 CrlApbClkCntrl;

	if (GemVersion == 2)
	{
		/*************************************/
		/* Setup device for first-time usage */
		/*************************************/

	/* SLCR unlock */
	*(volatile unsigned int *)(SLCR_UNLOCK_ADDR) = SLCR_UNLOCK_KEY_VALUE;
#ifndef __MICROBLAZE__
	if (EmacPsIntrId == XPS_GEM0_INT_ID) {
#ifdef XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV0
		/* GEM0 1G clock configuration*/
		SlcrTxClkCntrl =
		*(volatile unsigned int *)(SLCR_GEM0_CLK_CTRL_ADDR);
		SlcrTxClkCntrl &= EMACPS_SLCR_DIV_MASK;
		SlcrTxClkCntrl |= (XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV1 << 20);
		SlcrTxClkCntrl |= (XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV0 << 8);
		*(volatile unsigned int *)(SLCR_GEM0_CLK_CTRL_ADDR) =
								SlcrTxClkCntrl;
#endif
	} else if (EmacPsIntrId == XPS_GEM1_INT_ID) {
#ifdef XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV1
		/* GEM1 1G clock configuration*/
		SlcrTxClkCntrl =
		*(volatile unsigned int *)(SLCR_GEM1_CLK_CTRL_ADDR);
		SlcrTxClkCntrl &= EMACPS_SLCR_DIV_MASK;
		SlcrTxClkCntrl |= (XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV1 << 20);
		SlcrTxClkCntrl |= (XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV0 << 8);
		*(volatile unsigned int *)(SLCR_GEM1_CLK_CTRL_ADDR) =
								SlcrTxClkCntrl;
#endif
	}
#else
#ifdef XPAR_AXI_INTC_0_PROCESSING_SYSTEM7_0_IRQ_P2F_ENET0_INTR
	if (EmacPsIntrId == XPAR_AXI_INTC_0_PROCESSING_SYSTEM7_0_IRQ_P2F_ENET0_INTR) {
#ifdef XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV0
		/* GEM0 1G clock configuration*/
		SlcrTxClkCntrl =
		*(volatile unsigned int *)(SLCR_GEM0_CLK_CTRL_ADDR);
		SlcrTxClkCntrl &= EMACPS_SLCR_DIV_MASK;
		SlcrTxClkCntrl |= (XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV1 << 20);
		SlcrTxClkCntrl |= (XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV0 << 8);
		*(volatile unsigned int *)(SLCR_GEM0_CLK_CTRL_ADDR) =
								SlcrTxClkCntrl;
#endif
	}
#endif
#ifdef XPAR_AXI_INTC_0_PROCESSING_SYSTEM7_1_IRQ_P2F_ENET1_INTR
	if (EmacPsIntrId == XPAR_AXI_INTC_0_PROCESSING_SYSTEM7_1_IRQ_P2F_ENET1_INTR) {
#ifdef XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV1
		/* GEM1 1G clock configuration*/
		SlcrTxClkCntrl =
		*(volatile unsigned int *)(SLCR_GEM1_CLK_CTRL_ADDR);
		SlcrTxClkCntrl &= EMACPS_SLCR_DIV_MASK;
		SlcrTxClkCntrl |= (XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV1 << 20);
		SlcrTxClkCntrl |= (XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV0 << 8);
		*(volatile unsigned int *)(SLCR_GEM1_CLK_CTRL_ADDR) =
								SlcrTxClkCntrl;
#endif
	}
#endif
#endif
	/* SLCR lock */
	*(unsigned int *)(SLCR_LOCK_ADDR) = SLCR_LOCK_KEY_VALUE;
	#ifndef __MICROBLAZE__
	sleep(1);
	#else
	unsigned long count=0;
	while(count < 0xffff)
	{
		count++;
	}
	#endif
	}

	if ((GemVersion > 2)) {

#ifdef XPAR_PSU_ETHERNET_0_DEVICE_ID
		if (EmacPsIntrId == XPS_GEM0_INT_ID) {
			/* GEM0 1G clock configuration*/
			CrlApbClkCntrl =
			*(volatile unsigned int *)(CRL_GEM0_REF_CTRL);
			CrlApbClkCntrl &= ~CRL_GEM_DIV_MASK;
			CrlApbClkCntrl |= CRL_GEM_1G_DIV1;
			CrlApbClkCntrl |= CRL_GEM_1G_DIV0;
			*(volatile unsigned int *)(CRL_GEM0_REF_CTRL) =
									CrlApbClkCntrl;

		}
#endif
#ifdef XPAR_PSU_ETHERNET_1_DEVICE_ID
		if (EmacPsIntrId == XPS_GEM1_INT_ID) {

			/* GEM1 1G clock configuration*/
			CrlApbClkCntrl =
			*(volatile unsigned int *)(CRL_GEM1_REF_CTRL);
			CrlApbClkCntrl &= ~CRL_GEM_DIV_MASK;
			CrlApbClkCntrl |= CRL_GEM_1G_DIV1;
			CrlApbClkCntrl |= CRL_GEM_1G_DIV0;
			*(volatile unsigned int *)(CRL_GEM1_REF_CTRL) =
									CrlApbClkCntrl;
		}
#endif
#ifdef XPAR_PSU_ETHERNET_2_DEVICE_ID
		if (EmacPsIntrId == XPS_GEM2_INT_ID) {

			/* GEM2 1G clock configuration*/
			CrlApbClkCntrl =
			*(volatile unsigned int *)(CRL_GEM2_REF_CTRL);
			CrlApbClkCntrl &= ~CRL_GEM_DIV_MASK;
			CrlApbClkCntrl |= CRL_GEM_1G_DIV1;
			CrlApbClkCntrl |= CRL_GEM_1G_DIV0;
			*(volatile unsigned int *)(CRL_GEM2_REF_CTRL) =
									CrlApbClkCntrl;

		}
#endif
#ifdef XPAR_PSU_ETHERNET_3_DEVICE_ID
		if (EmacPsIntrId == XPS_GEM3_INT_ID) {
			/* GEM3 1G clock configuration*/
			CrlApbClkCntrl =
			*(volatile unsigned int *)(CRL_GEM3_REF_CTRL);
			CrlApbClkCntrl &= ~CRL_GEM_DIV_MASK;
			CrlApbClkCntrl |= CRL_GEM_1G_DIV1;
			CrlApbClkCntrl |= CRL_GEM_1G_DIV0;
			*(volatile unsigned int *)(CRL_GEM3_REF_CTRL) =
									CrlApbClkCntrl;
		}
#endif
	}
}

static XEmacPs_Bd *BdRxPtr;

int init_ethernet() {
	XEmacPs_Config *Config;
	XEmacPs_Bd BdTemplate;
	long Status;
	XEmacPs* EmacPsInstancePtr = &EmacPsInstance;
	XScuGic* IntcInstancePtr = &IntcInstance;

	Config = XEmacPs_LookupConfig(XPAR_XEMACPS_0_DEVICE_ID);
	Status = XEmacPs_CfgInitialize(EmacPsInstancePtr, Config, Config->BaseAddress);

	if (Status != XST_SUCCESS) {
		printf("EMAC: Error in initialize\n");
		return XST_FAILURE;
	}

	GemVersion = ((Xil_In32(Config->BaseAddress + 0xFC)) >> 16) & 0xFFF;
	printf("EMAC: GemVersion: %ld\n", GemVersion);

	XEmacPsClkSetup(EmacPsInstancePtr, EMACPS_IRPT_INTR);

	Status = XEmacPs_SetMacAddress(EmacPsInstancePtr, EmacPsMAC, 1);
	if (Status != XST_SUCCESS) {
		printf("EMAC: Error setting MAC address\n");
		return XST_FAILURE;
	}

	Status = XEmacPs_SetHandler(EmacPsInstancePtr,
					 XEMACPS_HANDLER_DMASEND,
					 (void *) XEmacPsSendHandler,
					 EmacPsInstancePtr);
	Status |=
		XEmacPs_SetHandler(EmacPsInstancePtr,
					XEMACPS_HANDLER_DMARECV,
					(void *) XEmacPsRecvHandler,
					EmacPsInstancePtr);
	Status |=
		XEmacPs_SetHandler(EmacPsInstancePtr, XEMACPS_HANDLER_ERROR,
					(void *) XEmacPsErrorHandler,
					EmacPsInstancePtr);

	if (Status != XST_SUCCESS) {
		printf("EMAC: Error assigning handlers\n");
		return XST_FAILURE;
	}

	// FIXME address space?
	/*
	 * The BDs need to be allocated in uncached memory. Hence the 1 MB
	 * address range that starts at address 0x0FF00000 is made uncached.
	 */
	Xil_SetTlbAttributes(RX_BD_LIST_START_ADDRESS, 0xc02);

	XEmacPs_BdClear(&BdTemplate);

	Status = XEmacPs_BdRingCreate(&(XEmacPs_GetRxRing
				       (EmacPsInstancePtr)),
				       RX_BD_LIST_START_ADDRESS,
				       RX_BD_LIST_START_ADDRESS,
				       XEMACPS_BD_ALIGNMENT,
				       RXBD_CNT);
	if (Status != XST_SUCCESS) {
		printf("EMAC: Error setting up RxBD space, BdRingCreate\n");
		return XST_FAILURE;
	}

	Status = XEmacPs_BdRingClone(&(XEmacPs_GetRxRing(EmacPsInstancePtr)),
				      &BdTemplate, XEMACPS_RECV);
	if (Status != XST_SUCCESS) {
		printf("EMAC: Error setting up RxBD space, BdRingClone\n");
		return XST_FAILURE;
	}

	XEmacPs_BdClear(&BdTemplate);
	XEmacPs_BdSetStatus(&BdTemplate, XEMACPS_TXBUF_USED_MASK);

	/*
	 * Create the TxBD ring
	 */
	Status = XEmacPs_BdRingCreate(&(XEmacPs_GetTxRing
				       (EmacPsInstancePtr)),
				       TX_BD_LIST_START_ADDRESS,
				       TX_BD_LIST_START_ADDRESS,
				       XEMACPS_BD_ALIGNMENT,
				       TXBD_CNT);
	if (Status != XST_SUCCESS) {
		printf("Error setting up TxBD space, BdRingCreate\n");
		return XST_FAILURE;
	}
	Status = XEmacPs_BdRingClone(&(XEmacPs_GetTxRing(EmacPsInstancePtr)), &BdTemplate, XEMACPS_SEND);
	if (Status != XST_SUCCESS) {
		printf("Error setting up TxBD space, BdRingClone\n");
		return XST_FAILURE;
	}

	XEmacPs_SetMdioDivisor(EmacPsInstancePtr, MDC_DIV_224);
	sleep(1);

	setup_phy(EmacPsInstancePtr, EMACPS_LOOPBACK_SPEED_1G);
	XEmacPs_SetOperatingSpeed(EmacPsInstancePtr, EMACPS_LOOPBACK_SPEED_1G);

	EmacPsSetupIntrSystem(IntcInstancePtr, EmacPsInstancePtr, EMACPS_IRPT_INTR);

	Status = XEmacPs_BdRingAlloc(&
				      (XEmacPs_GetRxRing(EmacPsInstancePtr)),
				      1, &BdRxPtr);
	if (Status != XST_SUCCESS) {
		printf("EMAC: Error allocating RxBD\n");
		return XST_FAILURE;
	}
	XEmacPs_BdSetAddressRx(BdRxPtr, RxFrame);
	Status = XEmacPs_BdRingToHw(&(XEmacPs_GetRxRing(EmacPsInstancePtr)), 1, BdRxPtr);
	if (Status != XST_SUCCESS) {
		printf("EMAC: Error committing RxBD to HW\n");
		return XST_FAILURE;
	}

	XEmacPs_Start(EmacPsInstancePtr);
	printf("EMAC: XEmacPs_Start done.\n");

	return XST_SUCCESS;
}

static void XEmacPsSendHandler(void *Callback)
{
	XEmacPs_Bd *BdTxPtr;
	XEmacPs *EmacPsInstancePtr = (XEmacPs *) Callback;

	u32 status = XEmacPs_ReadReg(EmacPsInstancePtr->Config.BaseAddress, XEMACPS_TXSR_OFFSET);
	XEmacPs_WriteReg(EmacPsInstancePtr->Config.BaseAddress, XEMACPS_TXSR_OFFSET, status);

	//printf("XEMACPS_TXSR status: %lu\n", status);

	XEmacPs_BdRingFromHwTx(&(XEmacPs_GetTxRing(EmacPsInstancePtr)), 1, &BdTxPtr);
	status = XEmacPs_BdRingFree(&(XEmacPs_GetTxRing(EmacPsInstancePtr)), 1, BdTxPtr);

	if (status != XST_SUCCESS) {
		printf("XEmacPs_BdRingFree error: %lu\n",status);
		return;
	}

    //XEmacPs_BdSetStatus(BdTxPtr, XEMACPS_TXBUF_WRAP_MASK|XEMACPS_TXBUF_USED_MASK);
    XEmacPs_BdSetStatus(BdTxPtr, XEMACPS_TXBUF_USED_MASK);

	FramesTx++;
}

#define XEMACPS_BD_TO_INDEX(ringptr, bdptr)				\
	(((u32)bdptr - (u32)(ringptr)->BaseBdAddr) / (ringptr)->Separation)

static volatile u16 frame_serial = 0;
static u32 frames_received = 0;

static void XEmacPsRecvHandler(void *Callback)
{
	XEmacPs* EmacPsInstancePtr = (XEmacPs *) Callback;

	XEmacPs_BdRing* rxring = &(XEmacPs_GetRxRing(EmacPsInstancePtr));
	XEmacPs_Bd* rxbdset, *cur_bd_ptr;

	int num_rx_bufs = XEmacPs_BdRingFromHwRx(rxring, 1, &rxbdset);

	// we immediately process the incoming frame
	// main task will then signal the Amiga via interrupt
	// driver on Amiga side will call ethernet_receive_frame after copying the frame
	// and this will free up the EmacPS receive buffer again.

	if (num_rx_bufs > 0) {
		//printf("EMAC: num_rx_bufs %d\n", num_rx_bufs);

		cur_bd_ptr = rxbdset;

		for (int i=0; i<num_rx_bufs; i++) {
			frame_serial++;

			u32 bd_idx = XEMACPS_BD_TO_INDEX(rxring, cur_bd_ptr);
			int rx_bytes = XEmacPs_BdGetLength(cur_bd_ptr);

			//printf("EMAC: RX: %d [%d] bd_idx: %d\n", frame_serial, rx_bytes, bd_idx);

			Xil_DCacheInvalidateRange((UINTPTR)RxFrame, sizeof(EthernetFrame));

			// store size in big endian
			*(RxFrame+RX_FRAME_PAD)   = (rx_bytes&0xff00)>>8;
			*(RxFrame+RX_FRAME_PAD+1) = (rx_bytes&0xff);
			*(RxFrame+RX_FRAME_PAD+2) = (frame_serial&0xff00)>>8;
			*(RxFrame+RX_FRAME_PAD+3) = (frame_serial&0xff);
			/*for (int i=0; i<rx_bytes; i++) {
				int y = i;
				printf("%02x",RxFrame[y]);
				if (y%4==3) printf(" ");
				if (y%32==31) printf("\n");
			}
			printf("\n==========================================\n");*/

			cur_bd_ptr = XEmacPs_BdRingNext(rxring, cur_bd_ptr);
		}

		frames_received++;

		int Status = XEmacPs_BdRingFree(rxring, 1, rxbdset);
		if (Status != XST_SUCCESS) {
			printf("EMAC: Error freeing RxBDs\n");
		}
	}

}

void ethernet_receive_frame() {
	XEmacPs* EmacPsInstancePtr = &EmacPsInstance;

	XEmacPs_BdRing* rxring = &(XEmacPs_GetRxRing(EmacPsInstancePtr));
	XEmacPs_Bd* rxbdset;

	int Status = XEmacPs_BdRingAlloc(rxring, 1, &rxbdset);
	if (Status != XST_SUCCESS) {
		printf("EMAC: Error allocating RxBD\n");
	} else {
		XEmacPs_BdSetAddressRx(rxbdset, RxFrame); // FIXME redundant?
		XEmacPs_BdClearRxNew(rxbdset);

		//XEmacPs_BdSetStatus(rxbdset, XEMACPS_RXBUF_WRAP_MASK);
		Status = XEmacPs_BdRingToHw(rxring, 1, rxbdset);
		if (Status != XST_SUCCESS) {
			printf("EMAC: Error committing RxBD to HW\n");
		}
	}

	u32 status = XEmacPs_ReadReg(EmacPsInstancePtr->Config.BaseAddress, XEMACPS_RXSR_OFFSET);
	XEmacPs_WriteReg(EmacPsInstancePtr->Config.BaseAddress, XEMACPS_RXSR_OFFSET, status);
}


u32 get_frames_received() {
	return frames_received;
}

static int frames_dropped = 0;

uint8_t* ethernet_get_mac_address_ptr() {
	return &EmacPsMAC;
}

void ethernet_update_mac_address() {
	XEmacPs* EmacPsInstancePtr = &EmacPsInstance;

	printf("Ethernet: New MAC address %x %x %x %x %x %x\n",
			EmacPsMAC[0],EmacPsMAC[1],EmacPsMAC[2],EmacPsMAC[3],EmacPsMAC[4],EmacPsMAC[5]);

	XEmacPs_Stop(EmacPsInstancePtr);

	int Status = XEmacPs_SetMacAddress(EmacPsInstancePtr, EmacPsMAC, 1);
	if (Status != XST_SUCCESS) {
		printf("EMAC: Error setting MAC address\n");
	}

	XEmacPs_Start(EmacPsInstancePtr);
}

static void XEmacPsErrorHandler(void *Callback, u8 Direction, u32 ErrorWord)
{
	//XEmacPs *EmacPsInstancePtr = (XEmacPs *) Callback;

	/*
	 * Increment the counter so that main thread knows something
	 * happened. Reset the device and reallocate resources ...
	 */
	DeviceErrors++;

	switch (Direction) {
	case XEMACPS_RECV:
		if (ErrorWord & XEMACPS_RXSR_HRESPNOK_MASK) {
			printf("EMAC: Receive DMA error\n");
		}
		if (ErrorWord & XEMACPS_RXSR_RXOVR_MASK) {
			printf("EMAC: Receive over run\n");
		}
		if (ErrorWord & XEMACPS_RXSR_BUFFNA_MASK) {
			//printf("EMAC: Receive buffer not available\n");
			// signal to host that frames are available
			frames_received++;
			frames_dropped++;
			//printf("#%d\n",frames_dropped);
		}
		break;
	case XEMACPS_SEND:
		if (ErrorWord & XEMACPS_TXSR_HRESPNOK_MASK) {
			printf("EMAC: Transmit DMA error\n");
		}
		if (ErrorWord & XEMACPS_TXSR_URUN_MASK) {
			printf("EMAC: Transmit under run\n");
		}
		if (ErrorWord & XEMACPS_TXSR_BUFEXH_MASK) {
			printf("EMAC: Transmit buffer exhausted\n");
		}
		if (ErrorWord & XEMACPS_TXSR_RXOVR_MASK) {
			printf("EMAC: Transmit retry excessed limits\n");
		}
		if (ErrorWord & XEMACPS_TXSR_FRAMERX_MASK) {
			printf("EMAC: Transmit collision\n");
		}
		if (ErrorWord & XEMACPS_TXSR_USEDREAD_MASK) {
			printf("EMAC: Transmit buffer not available\n");
		}
		break;
	}
	/*
	 * Bypassing the reset functionality as the default tx status for q0 is
	 * USED BIT READ. so, the first interrupt will be tx used bit and it resets
	 * the core always.
	 */
	if (GemVersion == 2) {
		//EmacPsResetDevice(EmacPsInstancePtr);
	}
}

LONG EmacPsUtilFrameVerify(EthernetFrame * CheckFrame, EthernetFrame * ActualFrame)
{
	char *CheckPtr = (char *) CheckFrame;
	char *ActualPtr = (char *) ActualFrame;
	u16 BytesLeft;
	u16 Counter;
	u32 Index;

	/*
	 * Compare the headers
	 */
	for (Index = 0; Index < XEMACPS_HDR_SIZE; Index++) {
		if (CheckPtr[Index] != ActualPtr[Index]) {
			return XST_FAILURE;
		}
	}

	/*
	 * Get the length of the payload
	 */
	BytesLeft = *(u16 *) &ActualPtr[12];
	/*
	 * Do endian swap from big back to little-endian.
	 */
	BytesLeft = Xil_EndianSwap16(BytesLeft);
	/*
	 * Validate the payload
	 */
	Counter = 0;
	ActualPtr = &ActualPtr[14];

	/*
	 * Check 8 bit incrementing pattern
	 */
	while (BytesLeft && (Counter < 256)) {
		if (*ActualPtr++ != (char) Counter++) {
			return XST_FAILURE;
		}
		BytesLeft--;
	}

	/*
	 * Check 16 bit incrementing pattern
	 */
	while (BytesLeft) {
		if (*ActualPtr++ != (char) (Counter >> 8)) {	/* high */
			return XST_FAILURE;
		}

		BytesLeft--;

		if (!BytesLeft)
			break;

		if (*ActualPtr++ != (char) Counter++) {	/* low */
			return XST_FAILURE;
		}

		BytesLeft--;
	}

	return XST_SUCCESS;
}

void EmacPsUtilFrameHdrFormatType(EthernetFrame * FramePtr, u16 FrameType)
{
	char *Frame = (char *) FramePtr;

	/*
	 * Increment to type field
	 */
	Frame = Frame + 12;
	/*
	 * Do endian swap from little to big-endian.
	 */
	FrameType = Xil_EndianSwap16(FrameType);
	/*
	 * Set the type
	 */
	*(u16 *) Frame = FrameType;
}

void EmacPsUtilFrameHdrFormatMAC(EthernetFrame * FramePtr, char *DestAddr)
{
	char *Frame = (char *) FramePtr;
	char *SourceAddress = EmacPsMAC;
	s32 Index;

	/* Destination address */
	for (Index = 0; Index < XEMACPS_MAC_ADDR_SIZE; Index++) {
		*Frame++ = *DestAddr++;
	}

	/* Source address */
	for (Index = 0; Index < XEMACPS_MAC_ADDR_SIZE; Index++) {
		*Frame++ = *SourceAddress++;
	}
}

void EmacPsUtilFrameSetPayloadData(EthernetFrame * FramePtr, u32 PayloadSize)
{
	u32 BytesLeft = PayloadSize;
	u8 *Frame;
	u16 Counter = 0;

	/*
	 * Set the frame pointer to the start of the payload area
	 */
	Frame = (u8 *) FramePtr + XEMACPS_HDR_SIZE;

	/*
	 * Insert 8 bit incrementing pattern
	 */
	while (BytesLeft && (Counter < 256)) {
		*Frame++ = (u8) Counter++;
		BytesLeft--;
	}

	/*
	 * Switch to 16 bit incrementing pattern
	 */
	while (BytesLeft) {
		*Frame++ = (u8) (Counter >> 8);	/* high */
		BytesLeft--;

		if (!BytesLeft)
			break;

		*Frame++ = (u8) Counter++;	/* low */
		BytesLeft--;
	}
}

void EmacPsUtilFrameMemClear(EthernetFrame * FramePtr)
{
	u32 *Data32Ptr = (u32 *) FramePtr;
	u32 WordsLeft = sizeof(EthernetFrame) / sizeof(u32);

	/* frame should be an integral number of words */
	while (WordsLeft--) {
		*Data32Ptr++ = 0xDEADBEEF;
	}
}

u32 XEmacPsDetectPHY(XEmacPs * EmacPsInstancePtr)
{
	u32 PhyAddr;
	u32 Status;
	u16 PhyReg1;
	u16 PhyReg2;

	for (PhyAddr = 0; PhyAddr <= 31; PhyAddr++) {
		Status = XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr,
					  PHY_DETECT_REG1, &PhyReg1);

		Status |= XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr,
					   PHY_DETECT_REG2, &PhyReg2);

		if ((Status == XST_SUCCESS) &&
		    (PhyReg1 > 0x0000) && (PhyReg1 < 0xffff) &&
		    (PhyReg2 > 0x0000) && (PhyReg2 < 0xffff)) {
			/* Found a valid PHY address */
			return PhyAddr;
		}
	}

	return PhyAddr;		/* default to 32(max of iteration) */
}

LONG setup_phy(XEmacPs * EmacPsInstancePtr, u32 Speed)
{
	u16 PhyIdentity;
	u32 PhyAddr;

	/*
	 * Detect the PHY address
	 */
	PhyAddr = XEmacPsDetectPHY(EmacPsInstancePtr);

	if (PhyAddr >= 32) {
		printf("EMAC: Error detecting PHY\n");
		return XST_FAILURE;
	}

	XEmacPs_PhyRead(EmacPsInstancePtr, PhyAddr, PHY_DETECT_REG1, &PhyIdentity);

	if (PhyIdentity == PHY_ID_MICREL_KSZ9031) {
		printf("EMAC: MICREL KSZ9031 PHY detected\n");
		micrel_auto_negotiate(EmacPsInstancePtr, PhyAddr);
		return XST_SUCCESS;
	}
	else {
		printf("EMAC: Unsupported PHY with id 0x%x\n",PhyIdentity);
	}

	return XST_FAILURE;
}

/* Advertisement control register. */
#define ADVERTISE_10HALF	0x0020  /* Try for 10mbps half-duplex  */
#define ADVERTISE_1000XFULL	0x0020  /* Try for 1000BASE-X full-duplex */
#define ADVERTISE_10FULL	0x0040  /* Try for 10mbps full-duplex  */
#define ADVERTISE_1000XHALF	0x0040  /* Try for 1000BASE-X half-duplex */
#define ADVERTISE_100HALF	0x0080  /* Try for 100mbps half-duplex */
#define ADVERTISE_1000XPAUSE	0x0080  /* Try for 1000BASE-X pause    */
#define ADVERTISE_100FULL	0x0100  /* Try for 100mbps full-duplex */
#define ADVERTISE_1000XPSE_ASYM	0x0100  /* Try for 1000BASE-X asym pause */
#define ADVERTISE_100BASE4	0x0200  /* Try for 100mbps 4k packets  */


#define ADVERTISE_100_AND_10	(ADVERTISE_10FULL | ADVERTISE_100FULL | ADVERTISE_10HALF | ADVERTISE_100HALF)
#define ADVERTISE_100		(ADVERTISE_100FULL | ADVERTISE_100HALF)
#define ADVERTISE_10		(ADVERTISE_10FULL | ADVERTISE_10HALF)

#define ADVERTISE_1000		0x0300

#define IEEE_ASYMMETRIC_PAUSE_MASK				0x0800
#define IEEE_PAUSE_MASK							0x0400
#define IEEE_AUTONEG_ERROR_MASK					0x8000

#define IEEE_CONTROL_REG_OFFSET					0
#define IEEE_STATUS_REG_OFFSET					1
#define IEEE_AUTONEGO_ADVERTISE_REG				4
#define IEEE_PARTNER_ABILITIES_1_REG_OFFSET		5
#define IEEE_PARTNER_ABILITIES_2_REG_OFFSET		8
#define IEEE_PARTNER_ABILITIES_3_REG_OFFSET		10
#define IEEE_1000_ADVERTISE_REG_OFFSET			9
#define IEEE_COPPER_SPECIFIC_CONTROL_REG		16
#define IEEE_SPECIFIC_STATUS_REG				17
#define IEEE_COPPER_SPECIFIC_STATUS_REG_2		19
#define IEEE_EXT_PHY_SPECIFIC_CONTROL_REG   	20
#define IEEE_CONTROL_REG_MAC					21
#define IEEE_PAGE_ADDRESS_REGISTER				22

#define IEEE_CTRL_1GBPS_LINKSPEED_MASK			0x2040
#define IEEE_CTRL_LINKSPEED_MASK				0x0040
#define IEEE_CTRL_LINKSPEED_1000M				0x0040
#define IEEE_CTRL_LINKSPEED_100M				0x2000
#define IEEE_CTRL_LINKSPEED_10M					0x0000
#define IEEE_CTRL_RESET_MASK					0x8000
#define IEEE_CTRL_AUTONEGOTIATE_ENABLE			0x1000
#define IEEE_STAT_AUTONEGOTIATE_CAPABLE			0x0008
#define IEEE_STAT_AUTONEGOTIATE_COMPLETE		0x0020
#define IEEE_STAT_AUTONEGOTIATE_RESTART			0x0200
#define IEEE_STAT_1GBPS_EXTENSIONS				0x0100
#define IEEE_AN1_ABILITY_MASK					0x1FE0
#define IEEE_AN3_ABILITY_MASK_1GBPS				0x0C00
#define IEEE_AN1_ABILITY_MASK_100MBPS			0x0380
#define IEEE_AN1_ABILITY_MASK_10MBPS			0x0060
#define IEEE_RGMII_TXRX_CLOCK_DELAYED_MASK		0x0030

#define IEEE_1000BASE_STATUS_REG 0x0a

static u32 micrel_auto_negotiate(XEmacPs *xemacpsp, u32 phy_addr)
{
	u16 temp;
	u16 control;
	u16 status;
	u16 status_speed;
	u32 timeout_counter = 0;
	u32 temp_speed;
	u32 phyregtemp;

	printf("PHY: Start Micrel PHY auto negotiation\n");

	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, &control);  //reg 0x04
	control |= IEEE_ASYMMETRIC_PAUSE_MASK;   //0x0800
	control |= IEEE_PAUSE_MASK;
	control |= ADVERTISE_100;
	control |= ADVERTISE_10;
	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_AUTONEGO_ADVERTISE_REG, control);

	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET, &control);
	control |= ADVERTISE_1000;
	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_1000_ADVERTISE_REG_OFFSET,control);


	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_COPPER_SPECIFIC_CONTROL_REG, &control);  //reg 0x0f
	control |= (7 << 12);
	control |= (1 << 11);
	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_COPPER_SPECIFIC_CONTROL_REG, control);

	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);        //reg 0x00
	control |= IEEE_CTRL_AUTONEGOTIATE_ENABLE;
	control |= IEEE_STAT_AUTONEGOTIATE_RESTART;
	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, control);

	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
	control |= IEEE_CTRL_RESET_MASK;
	XEmacPs_PhyWrite(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, control);

	while (1) {
		XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_CONTROL_REG_OFFSET, &control);
		if (control & IEEE_CTRL_RESET_MASK) continue; // ???
		else break;
	}

	XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_STATUS_REG_OFFSET, &status);

	printf("PHY: Waiting for PHY to complete auto negotiation.\n");

	while ( !(status & IEEE_STAT_AUTONEGOTIATE_COMPLETE) ) {
		sleep(1);
		XEmacPs_PhyRead(xemacpsp, phy_addr,
		IEEE_COPPER_SPECIFIC_STATUS_REG_2,  &temp);
		timeout_counter++;

		if (timeout_counter > 30) {
			printf("PHY: Auto negotiation timeout\n");
			return 0;
		}
		XEmacPs_PhyRead(xemacpsp, phy_addr, IEEE_STATUS_REG_OFFSET, &status);
	}
	printf("PHY: Auto negotiation complete\n");

	//XEmacPs_PhyRead(xemacpsp, phy_addr,IEEE_SPECIFIC_STATUS_REG, &status_speed);
	XEmacPs_PhyRead(xemacpsp, phy_addr,IEEE_1000BASE_STATUS_REG, &status_speed);
	if (status_speed & 0x0800) {
		printf("AUTO: PHY Speed: %x\n",status_speed);
		return 1000;
	}

	return 0;
}

/****************************************************************************/
/**
*
* This function setups the interrupt system so interrupts can occur for the
* EMACPS.
* @param	IntcInstancePtr is a pointer to the instance of the Intc driver.
* @param	EmacPsInstancePtr is a pointer to the instance of the EmacPs
*		driver.
* @param	EmacPsIntrId is the Interrupt ID and is typically
*		XPAR_<EMACPS_instance>_INTR value from xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
static LONG EmacPsSetupIntrSystem(XScuGic *IntcInstancePtr, XEmacPs *EmacPsInstancePtr, u16 EmacPsIntrId)
{
	LONG Status;
	XScuGic_Config *GicConfig;
	Xil_ExceptionInit();

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	GicConfig = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID);
	if (NULL == GicConfig) {
		printf("GIC: XScuGic_LookupConfig failed\n");
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, GicConfig, GicConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		printf("GIC: XScuGic_CfgInitialize failed\n");
		return XST_FAILURE;
	}

	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
			(Xil_ExceptionHandler)XScuGic_InterruptHandler,
			IntcInstancePtr);

	/*
	 * Connect a device driver handler that will be called when an
	 * interrupt for the device occurs, the device driver handler performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, EmacPsIntrId,
			(Xil_InterruptHandler) XEmacPs_IntrHandler,
			(void *) EmacPsInstancePtr);
	if (Status != XST_SUCCESS) {
		printf("GIC: Unable to connect ISR to interrupt controller\n");
		return XST_FAILURE;
	}

	/*
	 * Enable interrupts from the hardware
	 */
	XScuGic_Enable(IntcInstancePtr, EmacPsIntrId);
	printf("GIC: SCU GIC enabled\n");

	/*
	 * Enable interrupts in the processor
	 */
	Xil_ExceptionEnable();

	printf("GIC: Interrupts enabled\n");
	return XST_SUCCESS;
}

u16 ethernet_send_frame(u16 frame_size) {
	XEmacPs* EmacPsInstancePtr = &EmacPsInstance;
	XEmacPs_Bd *BdTxPtr;

	u32 old_frames_tx = FramesTx;

	//printf("ethernet_send_frame: %lu %d\n",old_frames_tx,frame_size);

	Xil_DCacheInvalidateRange((UINTPTR)TxFrame, sizeof(EthernetFrame));

	/*for (int i=0; i<frame_size; i++) {
		int y = i;
		printf("%02x",TxFrame[y]);
		if (y%4==3) printf(" ");
		if (y%32==31) printf("\n");
	}
	printf("\n==========================================\n");*/

	LONG Status = XEmacPs_BdRingAlloc(&(XEmacPs_GetTxRing(EmacPsInstancePtr)), 1, &BdTxPtr);

	if (Status != XST_SUCCESS) {
		printf("ERROR: BdRingAlloc error: %ld\n",Status);

		// lets unstick this
		XEmacPs_BdRingFree(&(XEmacPs_GetTxRing(EmacPsInstancePtr)), 1, BdTxPtr);
		return 2;
	}

	XEmacPs_BdSetAddressTx(BdTxPtr, TxFrame);
	XEmacPs_BdSetLength(BdTxPtr, frame_size);
	XEmacPs_BdClearTxUsed(BdTxPtr);
	XEmacPs_BdSetLast(BdTxPtr);

	Status = XEmacPs_BdRingToHw(&(XEmacPs_GetTxRing(EmacPsInstancePtr)), 1, BdTxPtr);

	if (Status != XST_SUCCESS) {
		printf("ERROR: BdRingToHw error: %ld\n",Status);
		return 3;
	}

	Xil_DCacheFlushRange((UINTPTR)BdTxPtr, 128);

	XEmacPs_Transmit(EmacPsInstancePtr);

	u32 counter = 0;
	while (old_frames_tx == FramesTx) {
		counter++;
		if (counter>1000) {
			printf("ERROR: timeout in ethernet_send_frame waiting for tx!\n");
			return 4;
		}
	}

	// all good
	return 0;
}

#endif
