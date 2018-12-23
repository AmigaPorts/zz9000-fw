/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xil_io.h"
#include "MNTZorro.h"

#include "xiicps.h"
#include "sleep.h"
#include "xaxivdma.h"
#include "xvtc.h"
#include "xil_cache.h"

typedef u8 AddressType;
#define IIC_DEVICE_ID	XPAR_XIICPS_0_DEVICE_ID
#define VDMA_DEVICE_ID	XPAR_AXI_VDMA_0_DEVICE_ID
#define HDMI_I2C_ADDR 	0x3b
#define IIC_SCLK_RATE	400000

XIicPs Iic;

int hdmi_ctrl_write_byte(u8 addr, u8 value) {
	u8 buffer[2];
	buffer[0] = addr;
	buffer[1] = value;
	int status;

	while (XIicPs_BusIsBusy(&Iic)) {};
	usleep(2500);
	status = XIicPs_MasterSendPolled(&Iic, buffer, 1, HDMI_I2C_ADDR);
	while (XIicPs_BusIsBusy(&Iic)) {};
	usleep(2500);
	buffer[1] = 0xff;
	status = XIicPs_MasterRecvPolled(&Iic, buffer+1, 1, HDMI_I2C_ADDR);

	printf("[hdmi] old value of 0x%0x: 0x%0x\n",addr,buffer[1]);
	buffer[1] = value;


	while (XIicPs_BusIsBusy(&Iic)) {};
	status = XIicPs_MasterSendPolled(&Iic, buffer, 2, HDMI_I2C_ADDR);

	while (XIicPs_BusIsBusy(&Iic)) {};
	usleep(2500);
	status = XIicPs_MasterSendPolled(&Iic, buffer, 1, HDMI_I2C_ADDR);

	while (XIicPs_BusIsBusy(&Iic)) {};
	usleep(2500);
	buffer[1] = 0xff;
	status = XIicPs_MasterRecvPolled(&Iic, buffer+1, 1, HDMI_I2C_ADDR);

	printf("[hdmi] new value of 0x%x: 0x%x (should be 0x%x)\n",addr,buffer[1],value);

	return status;
}

int hdmi_ctrl_read_byte(u8 addr, u8* buffer)
{
	buffer[0] = addr;
	buffer[1] = 0xff;
	while (XIicPs_BusIsBusy(&Iic)) {};
	int status = XIicPs_MasterSendPolled(&Iic, buffer, 1, HDMI_I2C_ADDR);
	while (XIicPs_BusIsBusy(&Iic)) {};
	usleep(2500);
	status = XIicPs_MasterRecvPolled(&Iic, buffer+1, 1, HDMI_I2C_ADDR);

	return status;
}


static const u8 sii9022_init[] = {
	0x1e, 0x00,	// TPI Device Power State Control Data (R/W)

	0x09, 0x00, //
	0x0a, 0x00,

	0x60, 0x04,
	0x3c, 0x01,	// TPI Interrupt Enable (R/W)

	0x1a, 0x10,	// TPI System Control (R/W)

	0x00, 0x4c,	// PixelClock/10000 - LSB
	0x01, 0x1d,	// PixelClock/10000 - MSB
	0x02, 0x70,	// Frequency in HZ - LSB
	0x03, 0x17,	// Vertical Frequency in HZ - MSB
	0x04, 0x70,	// Total Pixels per line - LSB
	0x05, 0x06,	// Total Pixels per line - MSB
	0x06, 0xEE,	// Total Lines - LSB
	0x07, 0x02,	// Total Lines - MSB
	0x08, 0x70, // pixel repeat rate?
	0x1a, 0x01, // DVI
};

void hdmi_ctrl_init() {
	int status;
	XIicPs_Config *config;
	config = XIicPs_LookupConfig(IIC_DEVICE_ID);
	status = XIicPs_CfgInitialize(&Iic, config, config->BaseAddress);
	printf("XIicPs_CfgInitialize: %d\n", status);
	status = XIicPs_SetSClk(&Iic, IIC_SCLK_RATE);
	printf("XIicPs_SetSClk: %d\n", status);

	Xil_Out32(0xE000A000 + 0x244,0x00080000);
	Xil_Out32(0xE000A000 + 0x248,0x00080000);
	Xil_Out32(0xE000A000 + 0xC,0xFFF70008);
	usleep(2500);

	status = hdmi_ctrl_write_byte(0xc7,0);

	u8 buffer[2];
	status = hdmi_ctrl_read_byte(0x1b,buffer);
	printf("[%d] TPI device id: 0x%x\n",status,buffer[1]);
	status = hdmi_ctrl_read_byte(0x1c,buffer);
	printf("[%d] TPI revision 1: 0x%x\n",status,buffer[1]);
	status = hdmi_ctrl_read_byte(0x1d,buffer);
	printf("[%d] TPI revision 2: 0x%x\n",status,buffer[1]);
	status = hdmi_ctrl_read_byte(0x30,buffer);
	printf("[%d] HDCP revision: 0x%x\n",status,buffer[1]);
	status = hdmi_ctrl_read_byte(0x3d,buffer);
	printf("[%d] hotplug: 0x%x\n",status,buffer[1]);

	for (int i=0; i<sizeof(sii9022_init); i+=2) {
		status = hdmi_ctrl_write_byte(sii9022_init[i], sii9022_init[i+1]);
		usleep(2500);
	}
}

void mandel(int k){
	float i,j,r,x,y=-16;
	while(puts(""),y++<15) for(x=0;x++<84;putchar(" .:-;!/>)|&IH%*#"[k&15]))for(i=k=r=0;
	j=r*r-i*i-2+x/25,i=2*r*i+y/10,j*j+i*i<11&&k++<111;r=j);
}

XAxiVdma vdma;
static u32* framebuffer=0;

int init_vdma(int hsize, int vsize) {
	int status;
	XAxiVdma_Config *Config;
	Config = XAxiVdma_LookupConfig(VDMA_DEVICE_ID);

	if (!Config) {
		printf("No video DMA found for ID %d\r\n", VDMA_DEVICE_ID);
		return XST_FAILURE;
	}
	status = XAxiVdma_CfgInitialize(&vdma, Config, Config->BaseAddress);
	if (status != XST_SUCCESS) {
		printf("Configuration Initialization failed, status: 0x%X\r\n", status);
		return status;
	}
	u32 stride = hsize * (Config->Mm2SStreamWidth>>3);

	XAxiVdma_DmaSetup ReadCfg;

	ReadCfg.VertSizeInput       = vsize;
	ReadCfg.HoriSizeInput       = stride;
	ReadCfg.Stride              = stride;
	ReadCfg.FrameDelay          = 0;      /* This example does not test frame delay */
	ReadCfg.EnableCircularBuf   = 1;      /* Only 1 buffer, continuous loop */
	ReadCfg.EnableSync          = 0;      /* Gen-Lock */
	ReadCfg.PointNum            = 0;
	ReadCfg.EnableFrameCounter  = 0;      /* Endless transfers */
	ReadCfg.FixedFrameStoreAddr = 0;      /* We are not doing parking */

	ReadCfg.FrameStoreStartAddr[0] = framebuffer;

	status = XAxiVdma_DmaConfig(&vdma, XAXIVDMA_READ, &ReadCfg);
	if (status != XST_SUCCESS) {
		printf("Read channel config failed, status: 0x%X\r\n", status);
		return status;
	}

	status = XAxiVdma_DmaSetBufferAddr(&vdma, XAXIVDMA_READ, ReadCfg.FrameStoreStartAddr);
	if (status != XST_SUCCESS) {
		printf("Read channel set buffer address failed, status: 0x%X\r\n", status);
		return status;
	}

	status = XAxiVdma_DmaStart(&vdma, XAXIVDMA_READ);
	if (status != XST_SUCCESS) {
		printf("Failed to start DMA engine (read channel), status: 0x%X\r\n", status);
		return status;
	}
	return XST_SUCCESS;
}


#define XVTC_DEVICE_ID			XPAR_VTC_0_DEVICE_ID
XVtc	VtcInst;

int init_vtc() {
	int status;
	XVtc_Config *Config;

	Config = XVtc_LookupConfig(XVTC_DEVICE_ID);

	if (NULL == Config) {
		xil_printf("Failed: XVtc_LookupConfig\r\n");
		return (XST_FAILURE);
	}
	printf("OK: XVtc_LookupConfig\n");

	status = XVtc_CfgInitialize(&VtcInst, Config, Config->BaseAddress);

	if (status != (XST_SUCCESS)) {
		xil_printf("Failed: XVtc_CfgInitialize\r\n");
		return (XST_FAILURE);
	}


	printf("OK: XVtc_CfgInitialize\n");

	status = XVtc_SelfTest(&VtcInst);

	if (status != (XST_SUCCESS)) {
		xil_printf("Failed: XVtc_SelfTest\r\n");
		return (XST_FAILURE);
	}

	printf("OK: XVtc_SelfTest\n");

	XVtc_EnableGenerator(&VtcInst);

	printf("OK: XVtc_EnableGenerator\n");

	return (XST_SUCCESS);
}


void dump_vtc_status()
{
	XVtc_Timing VideoTiming;

	XVtc_GetGeneratorTiming(&VtcInst, &VideoTiming);

	xil_printf("VTC timing:\n\r"
		"\tHActiveVideo : %d\n\r"
		"\tHFrontPorch  : %d\n\r"
		"\tHSyncWidth   : %d\n\r"
		"\tHBackPorch   : %d\n\r"
		"\tHSyncPolarity: %d\n\r"
		"\tVActiveVideo : %d\n\r"
		"\tV0FrontPorch : %d\n\r"
		"\tV0SyncWidth  : %d\n\r"
		"\tV0BackPorch  : %d\n\r"
		"\tV1FrontPorch : %d\n\r"
		"\tV1SyncWidth  : %d\n\r"
		"\tV1BackPorch  : %d\n\r"
		"\tVSyncPolarity: %d\n\r"
		"\tInterlaced   : %s\n\n\r\r",
		VideoTiming.HActiveVideo,
		VideoTiming.HFrontPorch,
		VideoTiming.HSyncWidth,
		VideoTiming.HBackPorch,
		VideoTiming.HSyncPolarity,
		VideoTiming.VActiveVideo,
		VideoTiming.V0FrontPorch,
		VideoTiming.V0SyncWidth,
		VideoTiming.V0BackPorch,
		VideoTiming.V1FrontPorch,
		VideoTiming.V1SyncWidth,
		VideoTiming.V1BackPorch,
		VideoTiming.VSyncPolarity,
	VideoTiming.Interlaced ? "Yes" : "No (Progressive)");
	xil_printf("\n\r");
}

u32 dump_vdma_status(XAxiVdma *InstancePtr)
{
    u32 status = XAxiVdma_GetStatus(InstancePtr, XAXIVDMA_READ);

	xil_printf("Read channel dump\n\r");
	xil_printf("\tMM2S DMA Control Register: %x\r\n",XAxiVdma_ReadReg(InstancePtr->ReadChannel.ChanBase, XAXIVDMA_CR_OFFSET));
	xil_printf("\tMM2S DMA Status Register: %x\r\n",XAxiVdma_ReadReg(InstancePtr->ReadChannel.ChanBase, XAXIVDMA_SR_OFFSET));
	xil_printf("\tMM2S HI_FRMBUF Reg: %x\r\n",XAxiVdma_ReadReg(InstancePtr->ReadChannel.ChanBase, XAXIVDMA_HI_FRMBUF_OFFSET));
	xil_printf("\tFRMSTORE Reg: %d\r\n",XAxiVdma_ReadReg(InstancePtr->ReadChannel.ChanBase, XAXIVDMA_FRMSTORE_OFFSET));
	xil_printf("\tBUFTHRES Reg: %d\r\n",XAxiVdma_ReadReg(InstancePtr->ReadChannel.ChanBase, XAXIVDMA_BUFTHRES_OFFSET));
	xil_printf("\tMM2S Vertical Size Register: %d\r\n",XAxiVdma_ReadReg(InstancePtr->ReadChannel.ChanBase, XAXIVDMA_MM2S_ADDR_OFFSET + XAXIVDMA_VSIZE_OFFSET));
	xil_printf("\tMM2S Horizontal Size Register: %d\r\n",XAxiVdma_ReadReg(InstancePtr->ReadChannel.ChanBase, XAXIVDMA_MM2S_ADDR_OFFSET + XAXIVDMA_HSIZE_OFFSET));
	xil_printf("\tMM2S Frame Delay and Stride Register: %d\r\n",XAxiVdma_ReadReg(InstancePtr->ReadChannel.ChanBase, XAXIVDMA_MM2S_ADDR_OFFSET + XAXIVDMA_STRD_FRMDLY_OFFSET));
	xil_printf("\tMM2S Start Address 1: %x\r\n",XAxiVdma_ReadReg(InstancePtr->ReadChannel.ChanBase, XAXIVDMA_MM2S_ADDR_OFFSET + XAXIVDMA_START_ADDR_OFFSET));

    if (status & XAXIVDMA_SR_HALTED_MASK) xil_printf("halted\n"); else xil_printf("running\n");
    if (status & XAXIVDMA_SR_IDLE_MASK) xil_printf("idle\n");
    if (status & XAXIVDMA_SR_ERR_INTERNAL_MASK) xil_printf("internal err\n");
    if (status & XAXIVDMA_SR_ERR_SLAVE_MASK) xil_printf("slave err\n");
    if (status & XAXIVDMA_SR_ERR_DECODE_MASK) xil_printf("decode err\n");
    if (status & XAXIVDMA_SR_ERR_FSZ_LESS_MASK) xil_printf("FSize Less Mismatch err\n");
    if (status & XAXIVDMA_SR_ERR_LSZ_LESS_MASK) xil_printf("LSize Less Mismatch err\n");
    if (status & XAXIVDMA_SR_ERR_SG_SLV_MASK) xil_printf("SG slave err\n");
    if (status & XAXIVDMA_SR_ERR_SG_DEC_MASK) xil_printf("SG decode err\n");
    if (status & XAXIVDMA_SR_ERR_FSZ_MORE_MASK) xil_printf("FSize More Mismatch err\n");

    return status;
}

static u32 fb_fill_offset=0;

void fb_fill() {
	for (int i=0; i<1280*720; i++) {
		framebuffer[i] = 0xffffffff-i/1280;
	}
	fb_fill_offset++;
}

int main()
{
	char* zstates[42] = {
		"RESET",
		"Z2_CONF",
		"Z2_IDLE",
		"WAIT_WRI",
		"WAIT_WRI2",
		"Z2WRIFIN",
		"WAIT_RD",
		"WAIT_RD2",
		"WAIT_RD3",
		"CONFIGURED",
		"CONF_CLEAR",
		"D_Z2_Z3",
		"Z3_IDLE",
		"Z3_WRITE_UPP",
		"Z3_WRITE_LOW",
		"Z3_READ_UP",
		"Z3_READ_LOW",
		"Z3_READ_DLY",
		"Z3_READ_DLY1",
		"Z3_READ_DLY2",
		"Z3_WRITE_PRE",
		"Z3_WRITE_FIN",
		"Z3_ENDCYCLE",
		"Z3_DTACK",
		"Z3_CONFIG",
		"Z2_REGWRITE",
		"REGWRITE",
		"REGREAD",
		"Z2_REGR_POST",
		"Z3_REGR_POST",
		"Z3_REGWRITE",
		"Z2_REGREAD",
		"Z3_REGREAD",
		"NONE_33",
		"Z2_PRE_CONF",
		"Z2_ENDCYCLE",
		"NONE_36",
		"NONE_37",
		"NONE_38",
		"RESET_DVID",
		"COLD",
		"UNDEF"
	};

    init_platform();

    framebuffer=(u8*)0x110000;

    mandel(0);

    //Xil_DCacheDisable();

    fb_fill();

    //MNTZORRO_mWriteReg(XPAR_MNTZORRO_0_S00_AXI_BASEADDR, MNTZORRO_S00_AXI_SLV_REG1_OFFSET, 0xbabecafe);

    // enable HDMI
    hdmi_ctrl_init();

    //printf("init_vtc()...\n");

    //init_vtc();

    printf("done. init_vdma()...\n");
    init_vdma(1280,720);
    printf("done.\n");

    usleep(10000);

    dump_vdma_status(&vdma);
    //dump_vtc_status();

    int old_zstate = 0;
    int need_req_ack = 0;

    uint16_t rect_x1=0;
    uint16_t rect_x2=0;
    uint16_t rect_y1=0;
    uint16_t rect_y2=0;
    uint16_t rect_rgb=0;

    while(1) {
        //uint32_t zbits = MNTZORRO_mReadReg(XPAR_MNTZORRO_0_S00_AXI_BASEADDR, MNTZORRO_S00_AXI_SLV_REG2_OFFSET);
        uint32_t zstate = MNTZORRO_mReadReg(XPAR_MNTZORRO_0_S00_AXI_BASEADDR, MNTZORRO_S00_AXI_SLV_REG3_OFFSET);

        u32 writereq = (zstate&(1<<31));
        u32 readreq = (zstate&(1<<30));

        uint32_t zstate_orig = zstate;
        zstate = zstate&0xf;
        if (zstate>40) zstate=41;

        //uint32_t zaddr = MNTZORRO_mReadReg(XPAR_MNTZORRO_0_S00_AXI_BASEADDR, MNTZORRO_S00_AXI_SLV_REG2_OFFSET);
        //printf("ADDR: %lx\n",zaddr);

        //if (zstate!=old_zstate) {
        	/*printf("addr: %08lx data: %04lx %s %s %s %s strb: %lx%lx%lx%lx %ld %s\r\n",zaddr,zdata,
    			(zbits&(1<<8))?"   ":"RST",
    			(zbits&(1<<5))?"RD":"  ",
				(zbits&(1<<6))?"   ":"CCS",
				(zbits&(1<<4))?"DOE":"   ",

				(zbits&(1<<3))>>3,
				(zbits&(1<<2))>>2,
				(zbits&(1<<1))>>1,
				zbits&(1<<0),
				zstate_orig,
				zstates[zstate]);*/

        //printf("ZSTA: %s write: %d read: %d data: %lx\n", zstates[zstate],!!writereq,!!readreq,zdata);

        	if (writereq) {
                uint32_t zaddr = MNTZORRO_mReadReg(XPAR_MNTZORRO_0_S00_AXI_BASEADDR, MNTZORRO_S00_AXI_SLV_REG0_OFFSET);
                uint32_t zdata = MNTZORRO_mReadReg(XPAR_MNTZORRO_0_S00_AXI_BASEADDR, MNTZORRO_S00_AXI_SLV_REG1_OFFSET);
        		printf("WRTE addr: %08lx data: %08lx\n",zaddr,zdata);

        		const uint32_t base=0x610000;

                if (zaddr==base) 	  rect_x1=zdata;
                else if (zaddr==base+2) rect_y1=zdata;
                else if (zaddr==base+4) rect_x2=zdata;
                else if (zaddr==base+6) rect_y2=zdata;
                else if (zaddr==base+8) {
                	rect_rgb=zdata;
                	uint32_t pitch=1280*2;
                	// fill rectangle
                	for (uint16_t y=rect_y1; y<=rect_y2; y++) {
                		for (uint16_t x=rect_x1; x<=rect_x2; x++) {
                			((u16*)framebuffer)[y*pitch+x]=rect_rgb;
                		}
                	}
                	Xil_DCacheFlush();
                }
                else if (zaddr>=0x610000) {
                	((u16*)framebuffer)[zaddr-0x610000] = zdata;
                	((u16*)framebuffer)[zaddr-0x610000+1] = zdata;
                }

        		// ack the write
        		MNTZORRO_mWriteReg(XPAR_MNTZORRO_0_S00_AXI_BASEADDR, MNTZORRO_S00_AXI_SLV_REG0_OFFSET, (1<<31));
        		need_req_ack = 1;
        	}
        	else if (readreq) {
                uint32_t zaddr = MNTZORRO_mReadReg(XPAR_MNTZORRO_0_S00_AXI_BASEADDR, MNTZORRO_S00_AXI_SLV_REG0_OFFSET);
        		//printf("READ addr: %08lx\n",zaddr);
        		need_req_ack = 1;
        		MNTZORRO_mWriteReg(XPAR_MNTZORRO_0_S00_AXI_BASEADDR, MNTZORRO_S00_AXI_SLV_REG1_OFFSET, ((u16*)framebuffer)[zaddr]);
        		MNTZORRO_mWriteReg(XPAR_MNTZORRO_0_S00_AXI_BASEADDR, MNTZORRO_S00_AXI_SLV_REG0_OFFSET, (1<<30));
        	}
        //}

        if (need_req_ack && !writereq && !readreq) {
    		MNTZORRO_mWriteReg(XPAR_MNTZORRO_0_S00_AXI_BASEADDR, MNTZORRO_S00_AXI_SLV_REG0_OFFSET, 0);
    		need_req_ack = 0;
        }

        old_zstate = zstate;
    	//usleep(100);

    	//char buffer[2];
    	//int status = hdmi_ctrl_read_byte(0x3d,buffer);
    	//printf("[%d] hotplug: 0x%x\n",status,buffer[1]);
    }

    cleanup_platform();
    return 0;
}
