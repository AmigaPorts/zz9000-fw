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
*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xil_io.h"

#include "xiicps.h"
#include "sleep.h"
#include "xaxivdma.h"
#include "xil_cache.h"
#include "xclk_wiz.h"
#include "xil_exception.h"

#include "gfx.h"
#include "ethernet.h"
#include "xgpiops.h"

#include "xil_misc_psreset_api.h"

typedef u8 uint8_t;

#define A9_CPU_RST_CTRL		(XSLCR_BASEADDR + 0x244)
#define A9_RST1_MASK 		0x00000002
#define A9_CLKSTOP1_MASK	0x00000020

#define XSLCR_LOCK_ADDR		(XSLCR_BASEADDR + 0x4)
#define XSLCR_LOCK_CODE		0x0000767B

#define IIC_DEVICE_ID	XPAR_XIICPS_0_DEVICE_ID
#define VDMA_DEVICE_ID	XPAR_AXIVDMA_0_DEVICE_ID
#define HDMI_I2C_ADDR 	0x3b
#define IIC_SCLK_RATE	400000
#define GPIO_DEVICE_ID		XPAR_XGPIOPS_0_DEVICE_ID

#define I2C_PAUSE 10

// I2C controller instance
XIicPs Iic;

int hdmi_ctrl_write_byte(u8 addr, u8 value) {
	u8 buffer[2];
	buffer[0] = addr;
	buffer[1] = value;
	int status;

	while (XIicPs_BusIsBusy(&Iic)) {};
	usleep(I2C_PAUSE);
	status = XIicPs_MasterSendPolled(&Iic, buffer, 1, HDMI_I2C_ADDR);
	while (XIicPs_BusIsBusy(&Iic)) {};
	usleep(I2C_PAUSE);
	buffer[1] = 0xff;
	status = XIicPs_MasterRecvPolled(&Iic, buffer+1, 1, HDMI_I2C_ADDR);

	//printf("[hdmi] old value of 0x%0x: 0x%0x\n",addr,buffer[1]);
	buffer[1] = value;

	while (XIicPs_BusIsBusy(&Iic)) {};
	status = XIicPs_MasterSendPolled(&Iic, buffer, 2, HDMI_I2C_ADDR);

	while (XIicPs_BusIsBusy(&Iic)) {};
	usleep(I2C_PAUSE);
	status = XIicPs_MasterSendPolled(&Iic, buffer, 1, HDMI_I2C_ADDR);

	while (XIicPs_BusIsBusy(&Iic)) {};
	usleep(I2C_PAUSE);
	buffer[1] = 0xff;
	status = XIicPs_MasterRecvPolled(&Iic, buffer+1, 1, HDMI_I2C_ADDR);

	if (buffer[1]!=value) {
		printf("[hdmi] new value of 0x%x: 0x%x (should be 0x%x)\n",addr,buffer[1],value);
	}

	return status;
}

int hdmi_ctrl_read_byte(u8 addr, u8* buffer)
{
	buffer[0] = addr;
	buffer[1] = 0xff;
	while (XIicPs_BusIsBusy(&Iic)) {};
	int status = XIicPs_MasterSendPolled(&Iic, buffer, 1, HDMI_I2C_ADDR);
	while (XIicPs_BusIsBusy(&Iic)) {};
	usleep(I2C_PAUSE);
	status = XIicPs_MasterRecvPolled(&Iic, buffer+1, 1, HDMI_I2C_ADDR);

	return status;
}

static u8 sii9022_init[] = {
	0x1e, 0x00,	// TPI Device Power State Control Data (R/W)

	0x09, 0x00, //
	0x0a, 0x00,

	0x60, 0x04,
	0x3c, 0x01,	// TPI Interrupt Enable (R/W)

	0x1a, 0x10,	// TPI System Control (R/W)

	0x00, 0x4c,	// PixelClock/10000 - LSB          u16:6
	0x01, 0x1d,	// PixelClock/10000 - MSB
	0x02, 0x70,	// Frequency in HZ - LSB
	0x03, 0x17,	// Vertical Frequency in HZ - MSB
	0x04, 0x70,	// Total Pixels per line - LSB
	0x05, 0x06,	// Total Pixels per line - MSB
	0x06, 0xEE,	// Total Lines - LSB
	0x07, 0x02,	// Total Lines - MSB
	0x08, 0x70, // pixel repeat rate?
	0x1a, 0x00, // CTRL_DATA - bit 1 causes 2 purple extra columns on DVI monitors (probably HDMI mode)
};

void disable_reset_out() {
    XGpioPs Gpio;
	XGpioPs_Config *ConfigPtr;
	ConfigPtr = XGpioPs_LookupConfig(GPIO_DEVICE_ID);
	XGpioPs_CfgInitialize(&Gpio, ConfigPtr, ConfigPtr->BaseAddr);
	int output_pin = 7;

	XGpioPs_SetDirectionPin(&Gpio, output_pin, 1);
	XGpioPs_SetOutputEnablePin(&Gpio, output_pin, 1);
	XGpioPs_WritePin(&Gpio, output_pin, 0);
	usleep(10000);
	XGpioPs_WritePin(&Gpio, output_pin, 1);

    print("GPIO reset disable done.\n\r");
}

void hdmi_ctrl_init() {
	int status;
	XIicPs_Config *config;
	config = XIicPs_LookupConfig(IIC_DEVICE_ID);
	status = XIicPs_CfgInitialize(&Iic, config, config->BaseAddress);
	//printf("XIicPs_CfgInitialize: %d\n", status);
	usleep(10000);
	//printf("XIicPs is ready: %lx\n", Iic.IsReady);

	status = XIicPs_SelfTest(&Iic);
	//printf("XIicPs_SelfTest: %x\n", status);

	status = XIicPs_SetSClk(&Iic, IIC_SCLK_RATE);
	//printf("XIicPs_SetSClk: %x\n", status);

	usleep(2500);

	// reset
	status = hdmi_ctrl_write_byte(0xc7,0);

	u8 buffer[2];
	status = hdmi_ctrl_read_byte(0x1b,buffer);
	printf("[%d] TPI device id: 0x%x\n",status,buffer[1]);
	status = hdmi_ctrl_read_byte(0x1c,buffer);
	//printf("[%d] TPI revision 1: 0x%x\n",status,buffer[1]);
	//status = hdmi_ctrl_read_byte(0x1d,buffer);
	//printf("[%d] TPI revision 2: 0x%x\n",status,buffer[1]);
	//status = hdmi_ctrl_read_byte(0x30,buffer);
	//printf("[%d] HDCP revision: 0x%x\n",status,buffer[1]);
	//status = hdmi_ctrl_read_byte(0x3d,buffer);
	printf("[%d] hotplug: 0x%x\n",status,buffer[1]);

	for (int i=0; i<sizeof(sii9022_init); i+=2) {
		status = hdmi_ctrl_write_byte(sii9022_init[i], sii9022_init[i+1]);
		usleep(1);
	}
}

XAxiVdma vdma;
static u32* framebuffer=0;
static u32  framebuffer_pan_offset=0;
static u32  blitter_dst_offset=0;
static u32  blitter_src_offset=0;
static u32  vmode_hsize=800, vmode_vsize=600, vmode_hdiv=1, vmode_vdiv=2;

// 32bit: hdiv=1, 16bit: hdiv=2, 8bit: hdiv=4, ...
int init_vdma(int hsize, int vsize, int hdiv, int vdiv) {
	int status;
	XAxiVdma_Config *Config;

	Config = XAxiVdma_LookupConfig(VDMA_DEVICE_ID);

	if (!Config) {
		printf("VDMA not found for ID %d\r\n", VDMA_DEVICE_ID);
		return XST_FAILURE;
	}

	/*XAxiVdma_DmaStop(&vdma, XAXIVDMA_READ);
	XAxiVdma_Reset(&vdma, XAXIVDMA_READ);
	XAxiVdma_ClearDmaChannelErrors(&vdma, XAXIVDMA_READ, XAXIVDMA_SR_ERR_ALL_MASK);*/

	status = XAxiVdma_CfgInitialize(&vdma, Config, Config->BaseAddress);
	if (status != XST_SUCCESS) {
		printf("VDMA Configuration Initialization failed, status: 0x%X\r\n", status);
		//return status;
	}

	u32 stride = hsize * (Config->Mm2SStreamWidth>>3);

	XAxiVdma_DmaSetup ReadCfg;

	printf("VDMA HDIV: %d VDIV: %d\n",hdiv,vdiv);

	ReadCfg.VertSizeInput       = vsize/vdiv;
	ReadCfg.HoriSizeInput       = stride/hdiv; // note: changing this breaks the output
	ReadCfg.Stride              = stride/hdiv; // note: changing this is not a problem
	ReadCfg.FrameDelay          = 0;      /* This example does not test frame delay */
	ReadCfg.EnableCircularBuf   = 1;      /* Only 1 buffer, continuous loop */
	ReadCfg.EnableSync          = 0;      /* Gen-Lock */
	ReadCfg.PointNum            = 0;
	ReadCfg.EnableFrameCounter  = 0;      /* Endless transfers */
	ReadCfg.FixedFrameStoreAddr = 0;      /* We are not doing parking */

	ReadCfg.FrameStoreStartAddr[0] = (u32)framebuffer+framebuffer_pan_offset;

	printf("VDMA Framebuffer at 0x%x\n", ReadCfg.FrameStoreStartAddr[0]);

	status = XAxiVdma_DmaConfig(&vdma, XAXIVDMA_READ, &ReadCfg);
	if (status != XST_SUCCESS) {
		printf("VDMA Read channel config failed, status: 0x%X\r\n", status);
		return status;
	}

	status = XAxiVdma_DmaSetBufferAddr(&vdma, XAXIVDMA_READ, ReadCfg.FrameStoreStartAddr);
	if (status != XST_SUCCESS) {
		printf("VDMA Read channel set buffer address failed, status: 0x%X\r\n", status);
		return status;
	}

	status = XAxiVdma_DmaStart(&vdma, XAXIVDMA_READ);
	if (status != XST_SUCCESS) {
		printf("VDMA Failed to start DMA engine (read channel), status: 0x%X\r\n", status);
		return status;
	}
	return XST_SUCCESS;
}

void hdmi_set_video_mode(u16 htotal, u16 vtotal, u32 pixelclock_hz, u16 vhz, u8 hdmi)
{
	/*
	 * SII9022 registers
	 *
	    0x00, 0x4c,	// PixelClock/10000 - LSB
		0x01, 0x1d,	// PixelClock/10000 - MSB
		0x02, 0x70,	// Frequency in HZ - LSB
		0x03, 0x17,	// Vertical Frequency in HZ - MSB
		0x04, 0x70,	// Total Pixels per line - LSB
		0x05, 0x06,	// Total Pixels per line - MSB
		0x06, 0xEE,	// Total Lines - LSB
		0x07, 0x02,	// Total Lines - MSB
		0x08, 0x70, // pixel repeat rate?
		0x1a, 0x00, // 0: DVI, 1: HDMI
	*/

	// see also https://github.com/torvalds/linux/blob/master/drivers/gpu/drm/bridge/sii902x.c#L358

	u8* sii_mode=sii9022_init+12;

	sii_mode[2*0+1] = pixelclock_hz/10000;
	sii_mode[2*1+1] = (pixelclock_hz/10000)>>8;
	sii_mode[2*2+1] = vhz*100;
	sii_mode[2*3+1] = (vhz*100)>>8;
	sii_mode[2*4+1] = htotal;
	sii_mode[2*5+1] = htotal>>8;
	sii_mode[2*6+1] = vtotal;
	sii_mode[2*7+1] = vtotal>>8;
	sii_mode[2*9+1] = hdmi;
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

	xil_printf("VDMA status: ");
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

void fb_fill(uint32_t offset) {
	memset(framebuffer+offset, 0, 1280*1024*4);
}

static XClk_Wiz clkwiz;

void pixelclock_init(int mhz) {
	XClk_Wiz_Config conf;
	XClk_Wiz_CfgInitialize(&clkwiz, &conf, XPAR_CLK_WIZ_0_BASEADDR);

	u32 phase = XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x20C);
	u32 duty = XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x210);
	u32 divide = XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x208);
	u32 muldiv = XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x200);

	u32 mul = 11;
	u32 div = 1;
	u32 otherdiv = 11;

	// Multiply/divide 100mhz fabric clock to desired pixel clock
	if (mhz==50) {
		mul = 15;
		div = 1;
		otherdiv = 30;
	} else if (mhz==40) {
		mul = 14;
		div = 1;
		otherdiv = 35;
	} else if (mhz==75) {
		mul = 15;
		div = 1;
		otherdiv = 20;
	} else if (mhz==65) {
		mul = 13;
		div = 1;
		otherdiv = 20;
	} else if (mhz==27) {
		mul = 27;
		div = 2;
		otherdiv = 50;
	} else if (mhz==150) {
		mul = 15;
		div = 1;
		otherdiv = 10;
	} else if (mhz==25) { // 25.205
		mul = 15;
		div = 1;
		otherdiv = 60;
	} else if (mhz==27) { // 25.205
		mul = 27;
		div = 2;
		otherdiv = 50;
	} else if (mhz==108) {
		mul = 54;
		div = 5;
		otherdiv = 10;
	}

	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x200, (mul<<8) | div);
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x208, otherdiv);

	// load configuration
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR,  0x25C, 0x00000003);
	//XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR,  0x25C, 0x00000001);

	phase = XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x20C);
	printf("CLK phase: %lu\n", phase);
	duty = XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x210);
	printf("CLK duty: %lu\n", duty);
	divide = XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x208);
	printf("CLK divide: %lu\n", divide);
	muldiv = XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x200);
	printf("CLK muldiv: %lu\n", muldiv);
}


// FIXME!
#define MNTZ_BASE_ADDR 0x43C00000

#define MNTZORRO_REG0 0
#define MNTZORRO_REG1 4
#define MNTZORRO_REG2 8
#define MNTZORRO_REG3 12

#define mntzorro_read(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))

#define mntzorro_write(BaseAddress, RegOffset, Data) \
  	Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))

void video_formatter_valign() {
	// vertical alignment
	mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG3, 1);
	usleep(1);
	mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG2, 0x80000000+0x5); // OP_VSYNC
	usleep(1);
	mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG3, 0);
	usleep(1);
	mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG2, 0x80000000); // NOP
	usleep(1);
	mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG2, 0); // NOP
	usleep(1);
}

#define VF_DLY ;
#define MNTVF_OP_MAX 6
#define MNTVF_OP_HS 7
#define MNTVF_OP_VS 8
#define MNTVF_OP_POLARITY 10
#define MNTVF_OP_SCALE 4
#define MNTVF_OP_DIMENSIONS 2
#define MNTVF_OP_COLORMODE 1

void video_formatter_write(uint32_t data, uint16_t op) {
	mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG3, data);
	VF_DLY;
	mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG2, 0x80000000|op); // OP_MAX (vmax | hmax)
	VF_DLY;
	mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG2, 0x80000000); // NOP
	VF_DLY;
	mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG2, 0); // clear
	VF_DLY;
	mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG3, 0); // clear
	VF_DLY;
}

void video_formatter_init(int scalemode, int colormode, int width, int height, int htotal, int vtotal, int hss, int hse, int vss, int vse, int polarity) {
	video_formatter_write((vtotal<<16)|htotal, MNTVF_OP_MAX);
	video_formatter_write((height<<16)|width, MNTVF_OP_DIMENSIONS);
	video_formatter_write((hss<<16)|hse, MNTVF_OP_HS);
	video_formatter_write((vss<<16)|vse, MNTVF_OP_VS);
	video_formatter_write(polarity, MNTVF_OP_POLARITY);
	video_formatter_write(scalemode, MNTVF_OP_SCALE);
	video_formatter_write(colormode, MNTVF_OP_COLORMODE);

	video_formatter_valign();
}

void video_system_init(int hres, int vres, int htotal, int vtotal, int mhz, int vhz, int hdiv, int vdiv, int hdmi) {

	printf("VSI: %d x %d [%d x %d] %d MHz %d Hz, hdiv: %d vdiv: %d\n",hres,vres,htotal,vtotal,mhz,vhz,hdiv,vdiv);

	printf("pixelclock_init()...\n");
	pixelclock_init(mhz);
	printf("...done.\n");

	printf("hdmi_set_video_mode()...\n");
	hdmi_set_video_mode(hres, vres, mhz, vhz, hdmi);

	printf("hdmi_ctrl_init()...\n");
	hdmi_ctrl_init();

	printf("init_vdma()...\n");
	init_vdma(hres, vres, hdiv, vdiv);
	printf("...done.\n");

    //dump_vdma_status(&vdma);
}

// Our address space is relative to the autoconfig base address (for example, it could be 0x600000)
#define MNT_REG_BASE    			0x000000
#define MNT_FB_BASE     			0x010000
#define MNT_BASE_MODE   			MNT_REG_BASE+0x02
#define MNT_BASE_VDIV   			MNT_REG_BASE+0x04
#define MNT_BASE_HSIZE 				MNT_REG_BASE+0x04
#define MNT_BASE_VSIZE 				MNT_REG_BASE+0x06
#define MNT_BASE_PAN_HI 			MNT_REG_BASE+0x0a
#define MNT_BASE_PAN_LO 			MNT_REG_BASE+0x0c
#define MNT_BASE_HDIV   			MNT_REG_BASE+0x0e
#define MNT_BASE_RECTOP 			MNT_REG_BASE+0x10
#define MNT_BASE_BLIT_SRC_HI 		MNT_REG_BASE+0x28
#define MNT_BASE_BLIT_SRC_LO 		MNT_REG_BASE+0x2a
#define MNT_BASE_BLIT_DST_HI 		MNT_REG_BASE+0x2c
#define MNT_BASE_BLIT_DST_LO 		MNT_REG_BASE+0x2e
#define MNT_BASE_BLITTER_COLORMODE 	MNT_REG_BASE+0x30
#define MNT_BASE_BLIT_SRC_PITCH		MNT_REG_BASE+0x32

#define MNT_BASE_ETH_TX			MNT_REG_BASE+0x80
#define MNT_BASE_ETH_RX			MNT_REG_BASE+0x82
#define MNT_BASE_ETH_MAC_HI		MNT_REG_BASE+0x84
#define MNT_BASE_ETH_MAC_HI2	MNT_REG_BASE+0x86
#define MNT_BASE_ETH_MAC_LO		MNT_REG_BASE+0x88
#define MNT_BASE_RUN_HI			MNT_REG_BASE+0x90
#define MNT_BASE_RUN_LO			MNT_REG_BASE+0x92
#define MNT_BASE_RUN_ARGC		MNT_REG_BASE+0x94
#define MNT_BASE_RUN_ARG0		MNT_REG_BASE+0x96
#define MNT_BASE_RUN_ARG1		MNT_REG_BASE+0x98
#define MNT_BASE_RUN_ARG2		MNT_REG_BASE+0x9a
#define MNT_BASE_RUN_ARG3		MNT_REG_BASE+0x9c
#define MNT_BASE_RUN_ARG4		MNT_REG_BASE+0x9e
#define MNT_BASE_RUN_ARG5		MNT_REG_BASE+0xa0
#define MNT_BASE_RUN_ARG6		MNT_REG_BASE+0xa2
#define MNT_BASE_RUN_ARG7   	MNT_REG_BASE+0xa4
#define MNT_BASE_EVENT_SERIAL	MNT_REG_BASE+0xb0
#define MNT_BASE_EVENT_CODE		MNT_REG_BASE+0xb2
#define MNT_BASE_FW_VERSION		MNT_REG_BASE+0xc0

#define REVISION_MAJOR 1
#define REVISION_MINOR 3

#define ZZVMODE_1280x720		0
#define ZZVMODE_800x600			1
#define ZZVMODE_640x480			2
#define ZZVMODE_1024x768		3
#define ZZVMODE_1280x1024		4
#define ZZVMODE_1920x1080_60 	5
#define ZZVMODE_720x576			6 // 50hz
#define ZZVMODE_1920x1080_50	7 // 50hz

void video_mode_init(int mode, int scalemode, int colormode) {
	int hres, vres, hmax, vmax, hstart, hend, vstart, vend, polarity, mhz, vhz, hdmi;

	int hdiv = 1, vdiv = 1;

	if (scalemode&1) hdiv = 2;
	if (scalemode&2) vdiv = 2;

	if (colormode==0) hdiv*=4;
	if (colormode==1) hdiv*=2;

	switch (mode) {
	case ZZVMODE_1280x720:
		hres = 1280;
		vres = 720;
		hstart = 1390;
		hend = 1430;
		hmax = 1650;
		vstart = 725;
		vend = 730;
		vmax = 750;
		polarity = 0;
		mhz = 75;
		vhz = 60;
		hdmi = 0;
		break;
	case ZZVMODE_800x600:
		hres = 800;
		vres = 600;
		hstart = 840;
		hend = 968;
		hmax = 1056;
		vstart = 601;
		vend = 605;
		vmax = 628;
		polarity = 0;
		mhz = 40;
		vhz = 60;
		hdmi = 0;
		break;
	case ZZVMODE_640x480:
		hres = 640;
		vres = 480;
		hstart = 656;
		hend = 752;
		hmax = 800;
		vstart = 490;
		vend = 492;
		vmax = 525;
		polarity = 0;
		mhz = 25;
		vhz = 60;
		hdmi = 0;
		break;
	case ZZVMODE_1024x768:
		hres = 1024;
		vres = 768;
		hstart = 1048;
		hend = 1184;
		hmax = 1344;
		vstart = 771;
		vend = 777;
		vmax = 806;
		polarity = 0;
		mhz = 65;
		vhz = 60;
		hdmi = 0;
		break;
	case ZZVMODE_1280x1024:
		hres = 1280;
		vres = 1024;
		hstart = 1328;
		hend = 1440;
		hmax = 1688;
		vstart = 1025;
		vend = 1028;
		vmax = 1066;
		polarity = 0;
		mhz = 108;
		vhz = 60;
		hdmi = 0;
		break;
	case ZZVMODE_1920x1080_50:
		hres = 1920;
		vres = 1080;
		hstart = 2448;
		hend = 2492;
		hmax = 2640;
		vstart = 1084;
		vend = 1089;
		vmax = 1125;
		polarity = 0;
		mhz = 150;
		vhz = 50;
		hdmi = 0;
		break;
	case ZZVMODE_1920x1080_60:
		hres = 1920;
		vres = 1080;
		hstart = 2008;
		hend = 2052;
		hmax = 2200;
		vstart = 1084;
		vend = 1089;
		vmax = 1125;
		polarity = 0;
		mhz = 150;
		vhz = 50;
		hdmi = 0;
		break;
	case ZZVMODE_720x576:
		hres = 720;
		vres = 576;
		hstart = 732;
		hend = 796;
		hmax = 864;
		vstart = 581;
		vend = 586;
		vmax = 625;
		polarity = 1;
		mhz = 27;
		vhz = 50;
		hdmi = 0;
		break;
	default:
		printf("Error: unknown mode\n");
		return;
	}

	video_system_init(hres, vres, hmax, vmax, mhz, vhz, hdiv, vdiv, hdmi);

	video_formatter_init(scalemode, colormode,
			hres, vres, hmax, vmax, hstart, hend, vstart, vend, polarity);

    vmode_hsize = hres;
    vmode_vsize = vres;
	vmode_vdiv = vdiv;
	vmode_hdiv = hdiv;
}

// this mode can be changed by amiga software to select a different resolution / framerate for
// native video capture
//static int videocap_video_mode = ZZVMODE_720x576;
//static int video_mode = ZZVMODE_720x576|2<<12|MNTVA_COLOR_32BIT<<8;
//static int default_pan_offset = 0x00e00000;

// default to more compatible 60hz mode
static int videocap_video_mode = ZZVMODE_800x600;
static int video_mode = ZZVMODE_800x600|2<<12|MNTVA_COLOR_32BIT<<8;
static int default_pan_offset = 0x00e00bd0;

void handle_amiga_reset() {
	if (videocap_video_mode == ZZVMODE_800x600) {
		default_pan_offset = 0x00e00bd0;
	} else {
		default_pan_offset = 0x00e00000;
	}

    framebuffer_pan_offset = default_pan_offset;
    fb_fill(framebuffer_pan_offset/4);

    printf("    _______________   ___   ___   ___  \n");
    printf("   |___  /___  / _ \\ / _ \\ / _ \\ / _ \\ \n");
    printf("      / /   / / (_) | | | | | | | | | |\n");
    printf("     / /   / / \\__, | | | | | | | | | |\n");
    printf("    / /__ / /__  / /| |_| | |_| | |_| |\n");
    printf("   /_____/_____|/_/  \\___/ \\___/ \\___/ \n\n");

	usleep(10000);

	// scalemode 2 (vertical doubling)
	video_mode_init(videocap_video_mode, 2, MNTVA_COLOR_32BIT);
	video_mode = videocap_video_mode|2<<12|MNTVA_COLOR_32BIT<<8;

    ethernet_init();
}

uint16_t arm_app_output_event_serial = 0;
uint16_t arm_app_output_event_code = 0;
char arm_app_output_event_ack = 0;
uint16_t arm_app_output_events_blocking = 0;
uint16_t arm_app_output_putchar_to_events = 0;
uint16_t arm_app_input_event_serial = 0;
uint16_t arm_app_input_event_code = 0;
char arm_app_input_event_ack = 0;

uint32_t arm_app_output_events_timeout = 100000;

void arm_app_put_event_code(uint16_t code) {
	arm_app_output_event_code = code;
	arm_app_output_event_ack = 0;
	arm_app_output_event_serial++;
}

char arm_app_output_event_acked() {
	return arm_app_output_event_ack;
}

void arm_app_set_output_events_blocking(char blocking) {
	arm_app_output_events_blocking = blocking;
}

void arm_app_set_output_putchar_to_events(char putchar_enabled) {
	arm_app_output_putchar_to_events = putchar_enabled;
}

uint16_t arm_app_get_event_serial() {
	return arm_app_input_event_serial;
}

uint16_t arm_app_get_event_code() {
	arm_app_input_event_ack = 1;
	return arm_app_input_event_code;
}

int __attribute__ ((visibility ("default"))) _putchar(char c) {
	if (arm_app_output_putchar_to_events) {
		if (arm_app_output_events_blocking) {
			for (uint32_t i=0; i<arm_app_output_events_timeout; i++) {
				usleep(1);
				if (arm_app_output_event_ack) break;
			}
		}
		arm_app_put_event_code(c);
	}
	return putchar(c);
}

struct ZZ9K_ENV {
  uint32_t api_version;
  uint32_t argv[8];
  uint32_t argc;

  int      (*fn_putchar)(char);
  void     (*fn_set_output_putchar_to_events)(char);
  void     (*fn_set_output_events_blocking)(char);
  void     (*fn_put_event_code)(uint16_t);
  uint16_t (*fn_get_event_serial)();
  uint16_t (*fn_get_event_code)();
  char     (*fn_output_event_acked)();
};

void arm_exception_handler(void *callback);
void arm_exception_handler_illinst(void *callback);

volatile struct ZZ9K_ENV arm_run_env;
volatile void (*core1_trampoline)(volatile struct ZZ9K_ENV* env);
volatile int core2_execute = 0;

#pragma GCC push_options
#pragma GCC optimize ("O1")
// core1_loop is executed on core1 (vs core0)
void core1_loop() {
	asm("mov	r0, r0");
	asm("mrc	p15, 0, r1, c1, c0, 2");		/* read cp access control register (CACR) into r1 */
	asm("orr	r1, r1, #(0xf << 20)");		/* enable full access for p10 & p11 */
	asm("mcr	p15, 0, r1, c1, c0, 2");		/* write back into CACR */

	// enable FPU
	asm("fmrx	r1, FPEXC");			/* read the exception register */
	asm("orr	r1,r1, #0x40000000");		/* set VFP enable bit, leave the others in orig state */
	asm("fmxr	FPEXC, r1");			/* write back the exception register */

	// enable flow prediction
	asm("mrc	p15,0,r0,c1,c0,0");		/* flow prediction enable */
	asm("orr	r0, r0, #(0x01 << 11)");		/* #0x8000 */
	asm("mcr	p15,0,r0,c1,c0,0");

	asm("mrc	p15,0,r0,c1,c0,1");		/* read Auxiliary Control Register */
	asm("orr	r0, r0, #(0x1 << 2)");		/* enable Dside prefetch */
	asm("orr	r0, r0, #(0x1 << 1)");		/* enable L2 Prefetch hint */
	asm("mcr	p15,0,r0,c1,c0,1");		/* write Auxiliary Control Register */

	// stack
	asm("mov sp, #0x06000000");

	volatile uint32_t* addr = 0;
	addr[0] = 0xe3e0000f; // mvn	r0, #15  -- loads 0xfffffff0
	addr[1] = 0xe590f000; // ldr	pc, [r0] -- jumps to the address in that address

	// FIXME these don't seem to do anything useful yet
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_RESET, (Xil_ExceptionHandler)arm_exception_handler, NULL);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_DATA_ABORT_INT, (Xil_ExceptionHandler)arm_exception_handler, NULL);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_PREFETCH_ABORT_INT, (Xil_ExceptionHandler)arm_exception_handler, NULL);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_UNDEFINED_INT, (Xil_ExceptionHandler)arm_exception_handler_illinst, NULL);

	while (1) {
		while (!core2_execute) {
			usleep(1);
		}
		core2_execute = 0;
		printf("[CPU1] executing at %p.\n",core1_trampoline);
		Xil_DCacheFlush();
		Xil_ICacheInvalidate();

		asm("push {r0-r12}");
		// FIXME HACK save our stack pointer in 0x10000
		asm("mov r0, #0x00010000");
		asm("str sp, [r0]");

		core1_trampoline(&arm_run_env);

		asm("mov r0, #0x00010000");
		asm("ldr sp, [r0]");
		asm("pop {r0-r12}");
	}
}
#pragma GCC pop_options

int main()
{
	const char* zstates[53] = {
		"RESET   ",
		"Z2_CONF ",
		"Z2_IDLE ",
		"WAIT_WRI",
		"WAIT_WR2",
		"Z2WRIFIN",
		"WAIT_RD ",
		"WAIT_RD2",
		"WAIT_RD3",
		"CONFIGED",
		"CONF_CLR",
		"D_Z2_Z3 ",
		"Z3_IDLE ",
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
		"WR2B",
		"WR2C",
		"Z3DMA1",
		"Z3DMA2",
		"Z3_AUTOCONF_RD",
		"Z3_AUTOCONF_WR",
		"Z3_AUTOCONF_RD_DLY",
		"Z3_AUTOCONF_RD_DLY2",
		"Z3_REGWRITE_PRE",
		"Z3_REGREAD_PRE",
		"Z3_WRITE_PRE2",
		"UNDEF",
	};

    init_platform();

    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_DATA_ABORT_INT, (Xil_ExceptionHandler)arm_exception_handler, NULL);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_PREFETCH_ABORT_INT, (Xil_ExceptionHandler)arm_exception_handler, NULL);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_UNDEFINED_INT, (Xil_ExceptionHandler)arm_exception_handler_illinst, NULL);

    disable_reset_out();

    // FIXME constant
    framebuffer=(u32*)0x00200000;
    int need_req_ack = 0;
	u8* mem = (u8*)framebuffer;

    // blitter etc
    uint16_t rect_x1 = 0;
    uint16_t rect_x2 = 0;
    uint16_t rect_x3 = 0;
    uint16_t rect_y1 = 0;
    uint16_t rect_y2 = 0;
    uint16_t rect_y3 = 0;
    uint16_t blitter_dst_pitch = 640;
    uint32_t rect_rgb = 0;
    uint32_t rect_rgb2 = 0;
    uint32_t blitter_colormode = MNTVA_COLOR_32BIT;
    uint16_t blitter_src_pitch = 0;

	uint16_t blitter_user1 = 0;
	uint16_t blitter_user2 = 0;
	uint16_t blitter_user3 = 0;
	uint16_t blitter_user4 = 0;

    // ARM app run environment
    arm_run_env.api_version = 1;
    arm_run_env.fn_putchar = _putchar;
    arm_run_env.fn_get_event_code = arm_app_get_event_code;
    arm_run_env.fn_get_event_serial = arm_app_get_event_serial;
    arm_run_env.fn_output_event_acked = arm_app_output_event_acked;
    arm_run_env.fn_put_event_code = arm_app_put_event_code;
    arm_run_env.fn_set_output_events_blocking = arm_app_set_output_events_blocking;
    arm_run_env.fn_set_output_putchar_to_events = arm_app_set_output_putchar_to_events;

    arm_run_env.argc = 0;
    uint32_t arm_run_address=0;

    // ethernet state
    uint32_t old_frames_received = 0;
    uint16_t ethernet_send_result = 0;

    // zorro state
    u32 old_zstate;
    u32 zstate_raw;
    int interlace_old = 0;

    handle_amiga_reset();

    printf("launch core1...\n");
    volatile uint32_t* core1_addr=(volatile uint32_t*)0xFFFFFFF0;
	*core1_addr = (uint32_t)core1_loop;
	// Place some machine code in strategic positions that will catch core1 if it crashes
	// FIXME: clean this up and turn into a debug handler / monitor
	volatile uint32_t* core1_addr2=(volatile uint32_t*)0x140; // catch 1
	core1_addr2[0] = 0xe3e0000f; // mvn	r0, #15  -- loads 0xfffffff0
	core1_addr2[1] = 0xe590f000; // ldr	pc, [r0] -- jumps to the address in that address

	core1_addr2=(volatile uint32_t*)0x100; // catch 2
	core1_addr2[0] = 0xe3e0000f; // mvn	r0, #15  -- loads 0xfffffff0
	core1_addr2[1] = 0xe590f000; // ldr	pc, [r0] -- jumps to the address in that address

	asm("sev");
	printf("core1 now idling.\n");

	int cache_counter = 0;
	int debug_counter = 0;
	int videocap_enabled_old = 1;
	uint32_t framebuffer_pan_offset_old = framebuffer_pan_offset;
	video_mode = 0x2200;

	int backlog_nag_counter = 0;

    while(1) {
		u32 zstate = mntzorro_read(MNTZ_BASE_ADDR, MNTZORRO_REG3);
		zstate_raw = zstate;
        u32 writereq   = (zstate&(1<<31));
        u32 readreq    = (zstate&(1<<30));

        zstate = zstate&0xff;
        if (zstate>52) zstate=52;

        //printf("%d\n",zdebug);

        /*if (zstate!=old_zstate) {
        	printf("addr: %08lx data: %04lx %s %s %s %s strb: %lx%lx%lx%lx %ld %s\r\n",0,0,
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

        //printf("addr: %08lx\r\n", MNTZORRO_mReadReg(MNTZ_BASE_ADDR, MNTZORRO_S00_AXI_SLV_REG0_OFFSET));

		/*if (zstate!=old_zstate) {
	        u32 z3 = (zstate_raw&(1<<25));
			uint32_t z3addr = MNTZORRO_mReadReg(MNTZ_BASE_ADDR, MNTZORRO_S00_AXI_SLV_REG2_OFFSET);
			printf("ZSTA: %s (%08lx) z3: %d wr: %d rd: %d addr: %08lx\n", zstates[zstate], zstate_raw, !!z3, !!writereq, !!readreq, z3addr);
			old_zstate=zstate;
		}*/

		if (writereq) {
			u32 zaddr = mntzorro_read(MNTZ_BASE_ADDR, MNTZORRO_REG0);
			u32 zdata  = mntzorro_read(MNTZ_BASE_ADDR, MNTZORRO_REG1);

	        u32 ds3 = (zstate_raw&(1<<29));
	        u32 ds2 = (zstate_raw&(1<<28));
	        u32 ds1 = (zstate_raw&(1<<27));
	        u32 ds0 = (zstate_raw&(1<<26));

	        //printf("WRTE: %08lx <- %08lx [%d%d%d%d]\n",zaddr,zdata,!!ds3,!!ds2,!!ds1,!!ds0);

			if (zaddr>0x10000000) {
				printf("ERRW illegal address %08lx\n",zaddr);
			}
			else if (zaddr>=MNT_FB_BASE || zaddr>=MNT_REG_BASE+0x2000) {
				u8* ptr = mem;

				if (zaddr>=MNT_FB_BASE) {
					ptr = mem+zaddr-MNT_FB_BASE;
				}
				else if (zaddr<MNT_REG_BASE+0x8000) {
					// FIXME remove
					ptr = (u8*)(RX_FRAME_ADDRESS+zaddr-(MNT_REG_BASE+0x2000));
					printf("ERXF write: %08lx\n",(u32)ptr);
				}
				else if (zaddr<MNT_REG_BASE+0x10000) {
					ptr = (u8*)(TX_FRAME_ADDRESS+zaddr-(MNT_REG_BASE+0x8000));
				}

				// FIXME cache this
				u32 z3 = (zstate_raw&(1<<25));

				if (z3) {
					if (ds3) ptr[0] = zdata>>24;
					if (ds2) ptr[1] = zdata>>16;
					if (ds1) ptr[2] = zdata>>8;
					if (ds0) ptr[3] = zdata;
				} else {
					// swap bytes
					if (ds1) ptr[0] = zdata>>8;
					if (ds0) ptr[1] = zdata;
				}
			}
			else if (zaddr>=MNT_REG_BASE && zaddr<MNT_FB_BASE) {
				// register area
				//printf("REGW: %08lx <- %08lx [%d%d%d%d]\n",zaddr,zdata,!!ds3,!!ds2,!!ds1,!!ds0);

		        u32 z3 = (zstate_raw&(1<<25));
				if (z3) {
					// convert 32bit to 16bit addresses
					if (ds3 && ds2) {
						zdata=zdata>>16;
					}
					else if (ds1 && ds0) {
						zdata=zdata&0xffff;
						zaddr+=2;
					}
					else {
						zaddr=0; // cancel
					}
				}
				//printf("CONV: %08lx <- %08lx\n",zaddr,zdata);

				// PANNING
				if (zaddr==MNT_BASE_PAN_HI) framebuffer_pan_offset=zdata<<16;
				else if (zaddr==MNT_BASE_PAN_LO) {
					framebuffer_pan_offset|=zdata;

					if (framebuffer_pan_offset != framebuffer_pan_offset_old) {
						init_vdma(vmode_hsize, vmode_vsize, vmode_hdiv, vmode_vdiv);
						video_formatter_valign();
						framebuffer_pan_offset_old = framebuffer_pan_offset;
					}
				}
				else if (zaddr==MNT_BASE_BLIT_SRC_HI) blitter_src_offset=zdata<<16;
				else if (zaddr==MNT_BASE_BLIT_SRC_LO) blitter_src_offset|=zdata;
				else if (zaddr==MNT_BASE_BLIT_DST_HI) blitter_dst_offset=zdata<<16;
				else if (zaddr==MNT_BASE_BLIT_DST_LO) blitter_dst_offset|=zdata;

				// RECTOP
				// FIXME refactor these magic numbers / constants
				else if (zaddr==MNT_BASE_RECTOP)   rect_x1=zdata;
				else if (zaddr==MNT_BASE_RECTOP+2) rect_y1=zdata;
				else if (zaddr==MNT_BASE_RECTOP+4) rect_x2=zdata;
				else if (zaddr==MNT_BASE_RECTOP+6) rect_y2=zdata;
				else if (zaddr==MNT_BASE_RECTOP+8) blitter_dst_pitch=zdata;

				else if (zaddr==MNT_BASE_RECTOP+0xa) rect_x3=zdata;
				else if (zaddr==MNT_BASE_RECTOP+0xc) rect_y3=zdata;

				else if (zaddr == MNT_BASE_RECTOP + 0x30) blitter_user1 = zdata;
				else if (zaddr == MNT_BASE_RECTOP + 0x32) blitter_user2 = zdata;
				else if (zaddr == MNT_BASE_RECTOP + 0x34) blitter_user3 = zdata;
				else if (zaddr == MNT_BASE_RECTOP + 0x36) blitter_user4 = zdata;

				else if (zaddr==MNT_BASE_RECTOP+0xe) {
					rect_rgb&=0xffff0000;
					rect_rgb|=(((zdata&0xff)<<8)|zdata>>8);
				}
				else if (zaddr==MNT_BASE_RECTOP+0x10) {
					rect_rgb&=0x0000ffff;
					rect_rgb|=(((zdata&0xff)<<8)|zdata>>8)<<16;
				}
				else if (zaddr==MNT_BASE_RECTOP+0x24) {
					rect_rgb2&=0xffff0000;
					rect_rgb2|=(((zdata&0xff)<<8)|zdata>>8);
				}
				else if (zaddr==MNT_BASE_RECTOP+0x26) {
					rect_rgb2&=0x0000ffff;
					rect_rgb2|=(((zdata&0xff)<<8)|zdata>>8)<<16;
				}
				else if (zaddr==MNT_BASE_RECTOP+0x12) {
					// fill rectangle
					set_fb((uint32_t*)((u32)framebuffer+blitter_dst_offset), blitter_dst_pitch);
					fill_rect(rect_x1, rect_y1, rect_x2, rect_y2, rect_rgb, blitter_colormode);
				}
				else if (zaddr==MNT_BASE_RECTOP+0x14) {
					// copy rectangle
					set_fb((uint32_t*)((u32)framebuffer+blitter_dst_offset), blitter_dst_pitch);

					switch (zdata) {
						case 1: // Regular BlitRect
							copy_rect(rect_x1, rect_y1, rect_x2, rect_y2, rect_x3, rect_y3, blitter_colormode, framebuffer, blitter_dst_pitch);
							break;
						case 2: // BlitRectNoMaskComplete
							copy_rect(rect_x1, rect_y1, rect_x2, rect_y2, rect_x3, rect_y3, blitter_colormode, (uint32_t*)((u32)framebuffer+blitter_src_offset), blitter_src_pitch);
							break;
						default:
							break;
					}
					Xil_DCacheFlush();
				}
				else if (zaddr==MNT_BASE_RECTOP+0x16) {
					// fill template

					uint8_t draw_mode = blitter_colormode>>8;
					uint8_t* tmpl_data = (uint8_t*)((u32)framebuffer+blitter_src_offset);
					set_fb((uint32_t*)((u32)framebuffer+blitter_dst_offset), blitter_dst_pitch);

					uint8_t bpp = 2*(blitter_colormode&0xff);
					if (bpp==0) bpp = 1;
					uint16_t loop_rows = 0;

					if (zdata&0x8000) {
						// pattern mode
						// TODO yoffset
						loop_rows = zdata&0xff;
					}

					if (loop_rows>0) {
						/*printf("fill_template:\n====================\n");
						printf("bpp: %d\n", bpp);
						printf("loop_rows: %d\n", loop_rows);
						printf("x:y1 - x:y2 %d:%d - %d:%d\n", rect_x1, rect_y1, rect_x2, rect_y2);
						printf("draw_mode: %d\n", draw_mode);
						printf("rect_rgb: %lx\n", rect_rgb);
						printf("rect_rgb2: %lx\n", rect_rgb2);
						printf("rect_x3: %d\n", rect_x3);
						printf("rect_y3: %d\n", rect_y3);
						printf("tmpl_data: %p\n", tmpl_data);
						printf("blitter_src_pitch: %d\n\n", blitter_src_pitch);*/
					}

					pattern_fill_rect((blitter_colormode & 0x0F), rect_x1, rect_y1, rect_x2, rect_y2, draw_mode, 0xff, rect_rgb, rect_rgb2, rect_x3, rect_y3, tmpl_data, blitter_src_pitch, loop_rows);
				}
				else if (zaddr == MNT_BASE_RECTOP + 0x28) {
					// Rect P2C
					uint8_t draw_mode = blitter_colormode >> 8;
					uint8_t planes = (zdata & 0xFF00) >> 8;
					uint8_t mask = (zdata & 0xFF);
					uint16_t num_rows = blitter_user1;
					uint8_t layer_mask = blitter_user2;
					uint8_t* bmp_data = (uint8_t*)((u32)framebuffer + blitter_src_offset);

					set_fb((uint32_t*)((u32)framebuffer+blitter_dst_offset), blitter_dst_pitch);

					p2c_rect(rect_x1, rect_y1, rect_x2, rect_y2, rect_x3, rect_y3, num_rows, draw_mode, planes, mask, layer_mask, blitter_src_pitch, bmp_data);
				}
				else if (zaddr == MNT_BASE_RECTOP + 0x2a) {
					// DrawLine
					uint8_t draw_mode = blitter_colormode >> 8;
					set_fb((uint32_t*) ((u32) framebuffer + blitter_dst_offset), blitter_dst_pitch);

					// rect_x3 contains the pattern. if all bits are set for both the mask and the pattern,
					// there's no point in passing non-essential data to the pattern/mask aware function.

					if (rect_x3 == 0xFFFF && zdata == 0xFF)
						draw_line_solid(rect_x1, rect_y1, rect_x2, rect_y2, rect_rgb, (blitter_colormode & 0x0F));
					else
						draw_line(rect_x1, rect_y1, rect_x2, rect_y2, rect_x3, rect_y3, rect_rgb, rect_rgb2, (blitter_colormode & 0x0F), zdata, draw_mode);
				}
				else if (zaddr == MNT_BASE_RECTOP + 0x2E) {
					// InvertRect
					set_fb((uint32_t*)((u32)framebuffer+blitter_dst_offset), blitter_dst_pitch);

					invert_rect(rect_x1, rect_y1, rect_x2, rect_y2, zdata & 0xFF, blitter_colormode);
				}
				else if (zaddr==MNT_BASE_BLITTER_COLORMODE) {
					blitter_colormode = zdata;
				}
				else if (zaddr==MNT_BASE_VDIV) {
					// retired
				}

				else if (zaddr==MNT_BASE_MODE) {
					printf("mode change: %x\n",zdata);

					if (video_mode != zdata) {
						int mode = zdata&0xff;
						int colormode = (zdata&0xf00)>>8;
						int scalemode = (zdata&0xf000)>>12;
						printf("mode: %d color: %d scale: %d\n",mode,colormode,scalemode);

						video_mode_init(mode, scalemode, colormode);
					}
					// remember selected video mode
					video_mode = zdata;
				}
				else if (zaddr==MNT_BASE_HSIZE) {
					// retired
				}
				else if (zaddr==MNT_BASE_VSIZE) {
					// retired
				}
				else if (zaddr==MNT_BASE_HDIV) {
					// retired
				}
				else if (zaddr==MNT_BASE_BLIT_SRC_PITCH) {
					blitter_src_pitch = zdata;
				}
				else if (zaddr==MNT_BASE_ETH_TX) {
					Xil_DCacheFlush();
					ethernet_send_result = ethernet_send_frame(zdata);
					//printf("SEND frame sz: %ld res: %d\n",zdata,ethernet_send_result);
				}
				else if (zaddr==MNT_BASE_ETH_RX) {
					//printf("RECV eth frame sz: %ld\n",zdata);
					ethernet_receive_frame();
				}
				else if (zaddr==MNT_BASE_ETH_MAC_HI) {
					uint8_t* mac = ethernet_get_mac_address_ptr();
					mac[0] = (zdata&0xff00)>>8;
					mac[1] = (zdata&0x00ff);
				}
				else if (zaddr==MNT_BASE_ETH_MAC_HI2) {
					uint8_t* mac = ethernet_get_mac_address_ptr();
					mac[2] = (zdata&0xff00)>>8;
					mac[3] = (zdata&0x00ff);
				}
				else if (zaddr==MNT_BASE_ETH_MAC_LO) {
					uint8_t* mac = ethernet_get_mac_address_ptr();
					mac[4] = (zdata&0xff00)>>8;
					mac[5] = (zdata&0x00ff);
					ethernet_update_mac_address();
				}
				else if (zaddr==MNT_BASE_RUN_HI) {
					arm_run_address=((u32)zdata)<<16;
				}
				else if (zaddr==MNT_BASE_RUN_LO) {
					// TODO checksum?
					arm_run_address|=zdata;

					*core1_addr  = (uint32_t)core1_loop;
					core1_addr2[0] = 0xe3e0000f; // mvn	r0, #15  -- loads 0xfffffff0
					core1_addr2[1] = 0xe590f000; // ldr	pc, [r0] -- jumps to the address in that address

					printf("[ARM_RUN] %lx\n",arm_run_address);
					if (arm_run_address>0) {
						core1_trampoline = (volatile void (*)(volatile struct ZZ9K_ENV*))arm_run_address;
						printf("[ARM_RUN] signaling second core.\n");
						Xil_DCacheFlush();
						Xil_ICacheInvalidate();
						core2_execute = 1;
						Xil_DCacheFlush();
						Xil_ICacheInvalidate();
					} else {
						core1_trampoline = 0;
						core2_execute = 0;
					}

					// FIXME move this out of here
					// sequence to reset cpu1 taken from https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/18842504/XAPP1079+Latest+Information

			    	Xil_Out32(XSLCR_UNLOCK_ADDR, XSLCR_UNLOCK_CODE);
			    	uint32_t RegVal = Xil_In32(A9_CPU_RST_CTRL);
			    	RegVal |= A9_RST1_MASK;
			    	Xil_Out32(A9_CPU_RST_CTRL, RegVal);
			    	RegVal |= A9_CLKSTOP1_MASK;
			    	Xil_Out32(A9_CPU_RST_CTRL, RegVal);
			    	RegVal &= ~A9_RST1_MASK;
					Xil_Out32(A9_CPU_RST_CTRL, RegVal);
			    	RegVal &= ~A9_CLKSTOP1_MASK;
					Xil_Out32(A9_CPU_RST_CTRL, RegVal);
			    	Xil_Out32(XSLCR_LOCK_ADDR, XSLCR_LOCK_CODE);

					dmb();
					dsb();
					isb();
					asm("sev");
				}
				else if (zaddr==MNT_BASE_RUN_ARGC) {
					arm_run_env.argc = zdata;
				}
				else if (zaddr==MNT_BASE_RUN_ARG0) {
					arm_run_env.argv[0] = ((u32)zdata)<<16;
				}
				else if (zaddr==MNT_BASE_RUN_ARG1) {
					arm_run_env.argv[0] |= zdata;
					printf("ARG0 set: %lx\n",arm_run_env.argv[0]);
				}
				else if (zaddr==MNT_BASE_RUN_ARG2) {
					arm_run_env.argv[1] = ((u32)zdata)<<16;
				}
				else if (zaddr==MNT_BASE_RUN_ARG3) {
					arm_run_env.argv[1] |= zdata;
					printf("ARG1 set: %lx\n",arm_run_env.argv[1]);
				}
				else if (zaddr==MNT_BASE_RUN_ARG4) {
					arm_run_env.argv[2] = ((u32)zdata)<<16;
				}
				else if (zaddr==MNT_BASE_RUN_ARG5) {
					arm_run_env.argv[2] |= zdata;
					printf("ARG2 set: %lx\n",arm_run_env.argv[2]);
				}
				else if (zaddr==MNT_BASE_RUN_ARG6) {
					arm_run_env.argv[3] = ((u32)zdata)<<16;
				}
				else if (zaddr==MNT_BASE_RUN_ARG7) {
					arm_run_env.argv[3] |= zdata;
					printf("ARG3 set: %lx\n",arm_run_env.argv[3]);
				}
				else if (zaddr==MNT_BASE_EVENT_CODE) {
					arm_app_input_event_code = zdata;
					arm_app_input_event_serial++;
					arm_app_input_event_ack = 0;
				}
			}

			// ack the write, set bit 31 in register 0
			mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG0, (1<<31));
			need_req_ack = 1;
		}
		else if (readreq) {
			uint32_t zaddr = mntzorro_read(MNTZ_BASE_ADDR, MNTZORRO_REG0);
			//printf("READ: %08lx\n",zaddr);
			u32 z3 = (zstate_raw&(1<<25)); // TODO cache

			if (zaddr>0x10000000) {
				printf("ERRR: illegal address %08lx\n",zaddr);
			}
			if (zaddr>=MNT_FB_BASE || zaddr>=MNT_REG_BASE+0x2000) {
				u8* ptr = mem;

				if (zaddr>=MNT_FB_BASE) {
					ptr = mem+zaddr-MNT_FB_BASE;
				}
				else if (zaddr<MNT_REG_BASE+0x8000) {
					// disable INT6 interrupt
					mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG2, (1<<30)|0);
					ptr = (u8*)(ethernet_current_receive_ptr()+zaddr-(MNT_REG_BASE+0x2000));
				}
				else if (zaddr<MNT_REG_BASE+0x10000) {
					ptr = (u8*)(TX_FRAME_ADDRESS+zaddr-(MNT_REG_BASE+0x8000));
					printf("ETXF read: %08lx\n",(u32)ptr);
				}

				if (z3) {
					u32 b1 = ptr[0]<<24;
					u32 b2 = ptr[1]<<16;
					u32 b3 = ptr[2]<<8;
					u32 b4 = ptr[3];
					mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG1, b1|b2|b3|b4);
				} else {
					u16 ubyte = ptr[0]<<8;
					u16 lbyte = ptr[1];
					mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG1, ubyte|lbyte);
				}
			}
			else if (zaddr>=MNT_REG_BASE) {
				// read ARM "register"
				uint32_t data = 0;
				uint32_t zaddr32 = zaddr&0xffffffc;

				if (zaddr32==MNT_BASE_EVENT_SERIAL) {
					data = (arm_app_output_event_serial<<16)|arm_app_output_event_code;
					arm_app_output_event_ack = 1;
				}
				else if (zaddr32==MNT_BASE_ETH_MAC_HI) {
					uint8_t* mac = ethernet_get_mac_address_ptr();
					data = mac[0]<<24|mac[1]<<16|mac[2]<<8|mac[3];
				}
				else if (zaddr32==MNT_BASE_ETH_MAC_LO) {
					uint8_t* mac = ethernet_get_mac_address_ptr();
					data = mac[4]<<24|mac[5]<<16;
				}
				else if (zaddr32==MNT_BASE_ETH_TX) {
					data = (ethernet_send_result&0xff)<<24|(ethernet_send_result&0xff00)<<16;
				}
				else if (zaddr32==MNT_BASE_FW_VERSION) {
					data = (REVISION_MAJOR<<24 | REVISION_MINOR<<16);
				}

				if (z3) {
					mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG1, data);
				} else {
					if (zaddr&2) {
						// lower 16 bit
						mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG1, data);
					} else {
						// upper 16 bit
						mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG1, data>>16);
					}
				}
			}

			// ack the read, set bit 30 in register 0
			mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG0, (1<<30));
			need_req_ack = 2;
		}
		else {
			// there are no read/write requests, we can do other housekeeping

			// 640x480x8
			// 10000: 5688 op/s
			// 25000: 5700-5900 op/s

			// we flush the cache at regular intervals to avoid too much visible cache activity on the screen
			// FIXME make this adjustable for user
			if (cache_counter>25000) {
				Xil_DCacheFlush();
				cache_counter = 0;
			}
			cache_counter++;

			int videocap_enabled = (zstate_raw&(1<<23));

			// FIXME magic constant
			if (videocap_enabled && framebuffer_pan_offset>=0xe00000) {
				int interlace = !!(zstate_raw&(1<<24));
				if (interlace != interlace_old) {
					// interlace has changed, we need to reconfigure vdma for the new screen height
					int vdiv = 2;
					if (interlace) {
						vdiv = 1;
					}
					// clear videocap area
					fb_fill(framebuffer_pan_offset/4);
					init_vdma(vmode_hsize,vmode_vsize,1,vdiv);
					video_formatter_valign();
					printf("videocap interlace mode changed.\n");
				}
				interlace_old = interlace;

				if (!videocap_enabled_old) {
					// remember current video mode as desired video capture video mode
					videocap_video_mode = video_mode&0xff;
				}
			}

			if (videocap_enabled_old != videocap_enabled) {
				if (framebuffer_pan_offset>=0xe00000) {
					// clear videocap area
					fb_fill(framebuffer_pan_offset/4);
				}
				videocap_enabled_old = videocap_enabled;
			}

			if (zstate==0) {
				// RESET
				// FIXME questionable
				handle_amiga_reset();
			}
		}

		// TODO: potential hang, timeout?
        if (need_req_ack) {
        	while (1) {
        		// 1. fpga needs to respond to flag bit 31 or 30 going high (signals request fulfilled)
        		// 2. it does that by clearing the request bit
        		// 3. we read register 3 until request bit (31:write, 30:read) goes to 0 again
        		//
        		u32 zstate = mntzorro_read(MNTZ_BASE_ADDR, MNTZORRO_REG3);
                u32 writereq   = (zstate&(1<<31));
                u32 readreq    = (zstate&(1<<30));
                if (need_req_ack==1 && !writereq) break;
                if (need_req_ack==2 && !readreq) break;
                if ((zstate&0xff) == 0) break; // reset
        	}
    		mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG0, 0);
    		need_req_ack = 0;
        }

		// check for ethernet frames
		int backlog = ethernet_get_backlog();
		if ((backlog>0 && backlog_nag_counter>5000)) {
			// interrupt amiga (trigger int6/2)
			mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG2, (1<<30)|1);
    		usleep(1);
    		mntzorro_write(MNTZ_BASE_ADDR, MNTZORRO_REG2, (1<<30)|0);
			backlog_nag_counter = 0;
		}

		if (backlog>0) {
			backlog_nag_counter++;
		}
    }

    cleanup_platform();
    return 0;
}

void arm_exception_handler(void *callback)
{
	printf("arm_exception_handler()!\n");
	while (1) {
	}
}

void arm_exception_handler_illinst(void *callback)
{
	printf("arm_exception_handler_illinst()!\n");
	while (1) {
	}
}
