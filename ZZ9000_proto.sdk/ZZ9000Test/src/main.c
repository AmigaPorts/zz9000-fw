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

#include "gfx.h"
#include "ethernet.h"
#include "xgpiops.h"

typedef u8 AddressType;
#define IIC_DEVICE_ID	XPAR_XIICPS_0_DEVICE_ID
#define VDMA_DEVICE_ID	XPAR_AXIVDMA_0_DEVICE_ID
#define HDMI_I2C_ADDR 	0x3b
#define IIC_SCLK_RATE	400000
#define GPIO_DEVICE_ID		XPAR_XGPIOPS_0_DEVICE_ID

#define MNTVA_COLOR_8BIT     0
#define MNTVA_COLOR_16BIT565 1
#define MNTVA_COLOR_32BIT    2
#define MNTVA_COLOR_1BIT     3
#define MNTVA_COLOR_15BIT    4

XIicPs Iic;

#define I2C_PAUSE 10000

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

	printf("[hdmi] old value of 0x%0x: 0x%0x\n",addr,buffer[1]);
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
	0x1a, 0x01, // DVI
};

void disable_reset_out() {
	int Status;
    XGpioPs Gpio;
	XGpioPs_Config *ConfigPtr;
	ConfigPtr = XGpioPs_LookupConfig(GPIO_DEVICE_ID);
	Status = XGpioPs_CfgInitialize(&Gpio, ConfigPtr, ConfigPtr->BaseAddr);
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
	printf("XIicPs_CfgInitialize: %d\n", status);
	usleep(10000);
	printf("iicps is ready: %x\n", Iic.IsReady);

	status = XIicPs_SelfTest(&Iic);
	printf("XIicPs_SelfTest: %x\n", status);

	status = XIicPs_SetSClk(&Iic, IIC_SCLK_RATE);
	printf("XIicPs_SetSClk: %x\n", status);

	// TODO what is this?
	//Xil_Out32(XPAR_PS7_GPIO_0_BASEADDR + 0x244,0x00080000);
	//Xil_Out32(XPAR_PS7_GPIO_0_BASEADDR + 0x248,0x00080000);
	//Xil_Out32(XPAR_PS7_GPIO_0_BASEADDR + 0xC,0xFFF70008);
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
static u32  framebuffer_pan_offset=0;
static u32  blitter_dst_offset=0;
static u32  blitter_src_offset=0;
static u32  vmode_hsize=640, vmode_vsize=480;

// 32bit: hdiv=1, 16bit: hdiv=2, 8bit: hdiv=4, ...
int init_vdma(int hsize, int vsize, int hdiv, int vdiv) {
	int status;
	XAxiVdma_Config *Config;

	Config = XAxiVdma_LookupConfig(VDMA_DEVICE_ID);

	if (!Config) {
		printf("No video DMA found for ID %d\r\n", VDMA_DEVICE_ID);
		return XST_FAILURE;
	}

	/*XAxiVdma_DmaStop(&vdma, XAXIVDMA_READ);
	XAxiVdma_Reset(&vdma, XAXIVDMA_READ);
	XAxiVdma_ClearDmaChannelErrors(&vdma, XAXIVDMA_READ, XAXIVDMA_SR_ERR_ALL_MASK);*/

	status = XAxiVdma_CfgInitialize(&vdma, Config, Config->BaseAddress);
	if (status != XST_SUCCESS) {
		printf("Configuration Initialization failed, status: 0x%X\r\n", status);
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

	printf("framebuffer set to %x\n", ReadCfg.FrameStoreStartAddr[0]);

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

void set_video_mode(u16 htotal, u16 vtotal, u32 pixelclock_hz, u16 vhz)
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
		0x1a, 0x01, // DVI
	*/

	u8* sii_mode=sii9022_init+12;

	sii_mode[2*0+1] = pixelclock_hz/10000;
	sii_mode[2*1+1] = (pixelclock_hz/10000)>>8;
	sii_mode[2*2+1] = vhz*100;
	sii_mode[2*3+1] = (vhz*100)>>8;
	sii_mode[2*4+1] = htotal;
	sii_mode[2*5+1] = htotal>>8;
	sii_mode[2*6+1] = vtotal;
	sii_mode[2*7+1] = vtotal>>8;
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

void fb_fill() {
	for (int i=0; i<720*576; i++) {
		framebuffer[i] = 0xff0000ff+i;
	}
}

static XClk_Wiz clkwiz;

void pixelclock_init(int mhz) {
	XClk_Wiz_Config conf;
	XClk_Wiz_CfgInitialize(&clkwiz, &conf, XPAR_CLK_WIZ_0_BASEADDR);

	u32 phase = XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x20C);
	printf("phase: %lu\n", phase);
	u32 duty = XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x210);
	printf("duty: %lu\n", duty);
	u32 divide = XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x208);
	printf("divide: %lu\n", divide);
	u32 muldiv = XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x200);
	printf("muldiv: %lu\n", muldiv);

	u32 mul = 11;
	u32 div = 1;
	u32 otherdiv = 11;

	// input 100mhz
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

	// 75mhz input
	/*if (mhz==50) {
		mul = 34;
		div = 3;
		otherdiv = 17;
	} else if (mhz==40) {
		mul = 40;
		div = 3;
		otherdiv = 25;
	} else if (mhz==75) {
		mul = 11;
		div = 1;
		otherdiv = 11;
	} else if (mhz==65) {
		mul = 13;
		div = 1;
		otherdiv = 15;
	} else if (mhz==27) {
		mul = 18;
		div = 1;
		otherdiv = 50;
	} else if (mhz==150) {
		mul = 12;
		div = 1;
		otherdiv = 6;
	} else if (mhz==25) { // 25.205
		mul = 41;
		div = 2;
		otherdiv = 61;
	} else if (mhz==108) {
		mul = 23;
		div = 2;
		otherdiv = 8;
	}*/

	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x200, (mul<<8) | div);
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x208, otherdiv);

	/*Writing this value sets CLKOUT0_DIVIDE to 5. The VCO frequency being 1000 MHz,
	dividing it by CLKOUT0_DIVIDE will give the 200 MHz frequency on the clkout1 in the IP.
 	Check for the status register, if the status register value is 0x1, then go to step 3*/

	// load configuration
	XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR,  0x25C, 0x00000003);
	//XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR,  0x25C, 0x00000001);

	phase = XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x20C);
	printf("phase: %lu\n", phase);
	duty = XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x210);
	printf("duty: %lu\n", duty);
	divide = XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x208);
	printf("divide: %lu\n", divide);
	muldiv = XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x200);
	printf("muldiv: %lu\n", muldiv);
}


#define MNTZORRO_S00_AXI_SLV_REG0_OFFSET 0
#define MNTZORRO_S00_AXI_SLV_REG1_OFFSET 4
#define MNTZORRO_S00_AXI_SLV_REG2_OFFSET 8
#define MNTZORRO_S00_AXI_SLV_REG3_OFFSET 12

#define MNTZORRO_mReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))

#define MNTZORRO_mWriteReg(BaseAddress, RegOffset, Data) \
  	Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))

void init_video_formatter(uint32_t base_addr, int colormode, int width, int height, int htotal, int vtotal, int hss, int hse, int vss, int vse, int polarity) {
	usleep(10000);
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG3_OFFSET, colormode);
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG2_OFFSET, 0xf0000001); // OP_COLORMODE
	usleep(10000);
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG2_OFFSET, 0xf0000000); // NOP
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG2_OFFSET, 0); // clear
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG3_OFFSET, 0); // clear
	usleep(10000);
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG3_OFFSET, (vtotal<<16)|htotal);
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG2_OFFSET, 0xf0000006); // OP_MAX (vmax | hmax)
	usleep(10000);
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG2_OFFSET, 0xf0000000); // NOP
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG2_OFFSET, 0); // clear
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG3_OFFSET, 0); // clear
	usleep(10000);
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG3_OFFSET, (height<<16)|width);
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG2_OFFSET, 0xf0000002); // OP_DIMENSIONS (height | width)
	usleep(10000);
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG2_OFFSET, 0xf0000000); // NOP
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG2_OFFSET, 0); // clear
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG3_OFFSET, 0); // clear
	usleep(10000);
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG3_OFFSET, (hss<<16)|hse);
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG2_OFFSET, 0xf0000007); // OP_HS (hsync_start | hsync_end)
	usleep(10000);
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG2_OFFSET, 0xf0000000); // NOP
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG2_OFFSET, 0); // clear
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG3_OFFSET, 0); // clear
	usleep(10000);
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG3_OFFSET, (vss<<16)|vse);
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG2_OFFSET, 0xf0000008); // OP_VS (vsync_start | vsync_end)
	usleep(10000);
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG2_OFFSET, 0xf0000000); // NOP
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG2_OFFSET, 0); // clear
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG3_OFFSET, 0); // clear
	usleep(10000);
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG3_OFFSET, polarity);
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG2_OFFSET, 0xf000000a); // OP_POLARITY
	usleep(10000);
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG2_OFFSET, 0xf0000000); // NOP
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG2_OFFSET, 0); // clear
	MNTZORRO_mWriteReg(base_addr, MNTZORRO_S00_AXI_SLV_REG3_OFFSET, 0); // clear
}

// FIXME!
#define MNTZ_BASE_ADDR 0x43C00000

void video_system_init(int hres, int vres, int htotal, int vtotal, int mhz, int vhz, int hdiv, int vdiv) {

    printf("pixelclock_init()...\n");
	pixelclock_init(mhz);

    printf("hdmi_ctrl_init()...\n");
    hdmi_ctrl_init();

    printf("set_video_mode()...\n");
    set_video_mode(htotal, vtotal, mhz, vhz);

    printf("init_vdma()...\n");
    init_vdma(hres, vres, hdiv, vdiv);
    printf("done.\n");

    // FIXME DEBUG ONLY
    /*printf("init_video_formatter()...\n");
    // 25.17 640 656 752 800 480 490 492 525
    if (hres==640 && vres==480) {
    	printf("640x480...\n");
    	init_video_formatter(MNTZ_BASE_ADDR, 2, hres, vres, htotal, vtotal, 656, 752, 490, 492, 0);
    }
    if (hres==1280 && vres==720) {
    	printf("1280x720...\n");
    	init_video_formatter(MNTZ_BASE_ADDR, 2, hres, vres, htotal, vtotal, 1720, 1760, 725, 730, 0);
    }
    printf("done.\n");*/

    dump_vdma_status(&vdma);

    vmode_hsize = hres;
    vmode_vsize = vres;

    /*while(1) {
    	usleep(100000000);
        dump_vdma_status(&vdma);
    }*/
}

// Our address space is relative to the autoconfig base address (for example, it could be 0x600000)
#define MNT_REG_BASE    0x000000
#define MNT_FB_BASE     0x010000
#define MNT_BASE_MODE   			MNT_REG_BASE+0x02
#define MNT_BASE_SCALEMODE   		MNT_REG_BASE+0x04
#define MNT_BASE_COLORMODE   		MNT_REG_BASE+0x0e
#define MNT_BASE_RECTOP 			MNT_REG_BASE+0x10
#define MNT_BASE_PAN_HI 			MNT_REG_BASE+0x0a
#define MNT_BASE_PAN_LO 			MNT_REG_BASE+0x0c
#define MNT_BASE_BLIT_SRC_HI 		MNT_REG_BASE+0x28
#define MNT_BASE_BLIT_SRC_LO 		MNT_REG_BASE+0x2a
#define MNT_BASE_BLIT_DST_HI 		MNT_REG_BASE+0x2c
#define MNT_BASE_BLIT_DST_LO 		MNT_REG_BASE+0x2e
#define MNT_BASE_BLITTER_COLORMODE 	MNT_REG_BASE+0x30
#define MNT_BASE_VIDEOCAP_MODE 		MNT_REG_BASE+0x32
#define MNT_BASE_HSIZE 		MNT_REG_BASE+0x34
#define MNT_BASE_VSIZE 		MNT_REG_BASE+0x36

#define MNT_BASE_ETH_TX		MNT_REG_BASE+0x80
#define MNT_BASE_RUN_HI		MNT_REG_BASE+0x90
#define MNT_BASE_RUN_LO		MNT_REG_BASE+0x92
#define MNT_BASE_RUN_ARG0   MNT_REG_BASE+0x94

void testfunc() {
	printf("testfunc!\n");
}

uint32_t mnt_alloc_ptr = 0x10000000;

uint8_t* mnt_malloc(uint32_t size) {
	uint8_t* res = (uint8_t*)mnt_alloc_ptr;
	if (mnt_alloc_ptr+size>0x20000000) {
		return 0;
	}
	mnt_alloc_ptr=(mnt_alloc_ptr+size+256)&0xffffff00;
	printf("mnt_malloc %ld at %p\n",size,res);
	return res;
}

void mnt_free(uint8_t* addr) {
	// NYI
}

void handle_amiga_reset() {
    fb_fill();

    framebuffer_pan_offset=0xe00000; //+0x1c200; // FIXME select sane offset and coordinate with verilog code

    printf("    _______________   ___   ___   ___  \n");
    printf("   |___  /___  / _ \\ / _ \\ / _ \\ / _ \\ \n");
    printf("      / /   / / (_) | | | | | | | | | |\n");
    printf("     / /   / / \\__, | | | | | | | | | |\n");
    printf("    / /__ / /__  / /| |_| | |_| | |_| |\n");
    printf("   /_____/_____|/_/  \\___/ \\___/ \\___/ \n\n");

    /*printf("mnt_malloc = %p;\n",mnt_malloc);
    printf("mnt_free = %p;\n",mnt_free);
    printf("memset = %p;\n",memset);
    printf("memcpy = %p;\n",memcpy);
    printf("printf = %p;\n",printf);
    printf("puts = %p;\n",puts);
    printf("sin = %p;\n",sin);
    printf("cos = %p;\n",cos);
    printf("sqrt = %p;\n",sqrt);*/

    // FIXME
    //video_system_init(640, 480, 800, 525, 25, 60, 1, 2);
    video_system_init(720, 576, 864, 625, 27, 50, 1, 2); // <--- default
    //video_system_init(1280, 720, 1980, 750, 75, 60, 1, 2);
}

int main()
{
	char* zstates[49] = {
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
		"UNDEF",
	};

    init_platform();
    Xil_DCacheDisable();

    disable_reset_out();

    framebuffer=(u32*)0x110000;
    int need_req_ack = 0;
	u8* mem = (u8*)framebuffer;

    // registers

    uint16_t rect_x1=0;
    uint16_t rect_x2=100;
    uint16_t rect_x3=100;
    uint16_t rect_y1=0;
    uint16_t rect_y2=0;
    uint16_t rect_y3=10;
    uint16_t rect_pitch=640;
    uint32_t rect_rgb=0;
    uint32_t blitter_colormode=MNTVA_COLOR_32BIT;
    uint16_t hdiv=1, vdiv=1;
    uint32_t arm_run_address=0;
    uint16_t arm_run_arg0=0;

    u32 old_zstate;
    u32 zstate_raw;
    int interlace_old = 0;

    handle_amiga_reset();

    //printf("init_ethernet...\n");
    //init_ethernet();
    //printf("... init_ethernet done.\n");

    while(1) {
		u32 zstate = MNTZORRO_mReadReg(MNTZ_BASE_ADDR, MNTZORRO_S00_AXI_SLV_REG3_OFFSET);
		zstate_raw = zstate;
        u32 writereq   = (zstate&(1<<31));
        u32 readreq    = (zstate&(1<<30));

        zstate = zstate&0xff;
        if (zstate>48) zstate=48;

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

		if (zstate!=old_zstate) {
	        u32 z3 = (zstate_raw&(1<<25));
			uint32_t z3addr = MNTZORRO_mReadReg(MNTZ_BASE_ADDR, MNTZORRO_S00_AXI_SLV_REG2_OFFSET);
			printf("ZSTA: %s (%08lx) z3: %d wr: %d rd: %d addr: %08lx\n", zstates[zstate], zstate_raw, !!z3, !!writereq, !!readreq, z3addr);
			old_zstate=zstate;
		}

		if (writereq) {
			u32 zaddr = MNTZORRO_mReadReg(MNTZ_BASE_ADDR, MNTZORRO_S00_AXI_SLV_REG0_OFFSET);
			u32 zdata  = MNTZORRO_mReadReg(MNTZ_BASE_ADDR, MNTZORRO_S00_AXI_SLV_REG1_OFFSET);
			//uint32_t count_writes = MNTZORRO_mReadReg(XPAR_MNTZORRO_0_S00_AXI_BASEADDR, MNTZORRO_S00_AXI_SLV_REG2_OFFSET);

	        u32 ds3 = (zstate_raw&(1<<29));
	        u32 ds2 = (zstate_raw&(1<<28));
	        u32 ds1 = (zstate_raw&(1<<27));
	        u32 ds0 = (zstate_raw&(1<<26));

	        //if (!(ds3 && ds2) && !(ds1 && ds0)) {
	        //	printf("ERR WRTE: %08lx <- %08lx [%d%d%d%d]\n",zaddr,zdata,!!ds3,!!ds2,!!ds1,!!ds0);
	        //}

			if (zaddr>0x10000000) {
				//printf("ERRW illegal address %p\n",zaddr);
			}
			else if (zaddr>=MNT_REG_BASE && zaddr<MNT_FB_BASE) {
				// register area

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
						printf("ERR REGW: %08lx <- %08lx [%d%d%d%d]\n",zaddr,zdata,!!ds3,!!ds2,!!ds1,!!ds0);
						zaddr=0; // cancel
					}
				}
				//printf("CONV: %08lx <- %08lx\n",zaddr,zdata);

				// PANNING
				if (zaddr==MNT_BASE_PAN_HI) framebuffer_pan_offset=zdata<<16;
				else if (zaddr==MNT_BASE_PAN_LO) {
					framebuffer_pan_offset|=zdata;
					init_vdma(vmode_hsize,vmode_vsize,hdiv,vdiv);
				}
				else if (zaddr==MNT_BASE_BLIT_SRC_HI) blitter_src_offset=zdata<<16;
				else if (zaddr==MNT_BASE_BLIT_SRC_LO) blitter_src_offset|=zdata;
				else if (zaddr==MNT_BASE_BLIT_DST_HI) blitter_dst_offset=zdata<<16;
				else if (zaddr==MNT_BASE_BLIT_DST_LO) blitter_dst_offset|=zdata;

				// RECTOP
				else if (zaddr==MNT_BASE_RECTOP)   rect_x1=zdata;
				else if (zaddr==MNT_BASE_RECTOP+2) rect_y1=zdata;
				else if (zaddr==MNT_BASE_RECTOP+4) rect_x2=zdata;
				else if (zaddr==MNT_BASE_RECTOP+6) rect_y2=zdata;
				else if (zaddr==MNT_BASE_RECTOP+8) rect_pitch=zdata;

				else if (zaddr==MNT_BASE_RECTOP+0xa) rect_x3=zdata;
				else if (zaddr==MNT_BASE_RECTOP+0xc) rect_y3=zdata;
				else if (zaddr==MNT_BASE_RECTOP+0xe) {
					rect_rgb&=0xffff0000;
					rect_rgb|=(((zdata&0xff)<<8)|zdata>>8);
				}
				else if (zaddr==MNT_BASE_RECTOP+0x10) {
					rect_rgb&=0x0000ffff;
					rect_rgb|=(((zdata&0xff)<<8)|zdata>>8)<<16;
				}
				else if (zaddr==MNT_BASE_RECTOP+0x12) {
					// fill rectangle

					//printf("FILL: %d %d %d %d %08lx\n",rect_x1,rect_y1,rect_x2,rect_y2,rect_rgb);

					set_fb((uint32_t*)((u32)framebuffer+blitter_dst_offset), rect_pitch);

					if (blitter_colormode==MNTVA_COLOR_16BIT565) {
						fill_rect16(rect_x1,rect_y1,rect_x2,rect_y2,rect_rgb);
					} else if (blitter_colormode==MNTVA_COLOR_8BIT) {
						fill_rect8(rect_x1,rect_y1,rect_x2,rect_y2,rect_rgb>>24);
					} else if (blitter_colormode==MNTVA_COLOR_32BIT) {
						fill_rect32(rect_x1,rect_y1,rect_x2,rect_y2,rect_rgb);
					}

					//Xil_DCacheFlush();
				}
				else if (zaddr==MNT_BASE_RECTOP+0x14) {
					set_fb((uint32_t*)((u32)framebuffer+blitter_dst_offset), rect_pitch);

					// copy rectangle
					if (blitter_colormode==MNTVA_COLOR_16BIT565) {
						// 16 bit
						copy_rect16(rect_x1,rect_y1,rect_x2,rect_y2,rect_x3,rect_y3);
					} else if (blitter_colormode==MNTVA_COLOR_8BIT) {
						// 8 bit
						copy_rect8(rect_x1,rect_y1,rect_x2,rect_y2,rect_x3,rect_y3);
					} else {
						// 32 bit
						copy_rect32(rect_x1,rect_y1,rect_x2,rect_y2,rect_x3,rect_y3);
					}
				}
				else if (zaddr==MNT_BASE_RECTOP+0x16) {
					// fill triangle
					Vec2 a = {rect_x1, rect_y1};
					Vec2 b = {rect_x2, rect_y2};
					Vec2 c = {rect_x3, rect_y3};
					set_fb((uint32_t*)((u32)framebuffer+blitter_dst_offset), rect_pitch);
					fill_triangle(a,b,c,rect_rgb);
				}
				else if (zaddr==MNT_BASE_BLITTER_COLORMODE) {
					// blitter_colormode
					//set_fb((uint32_t*)((u32)framebuffer+blitter_dst_offset), rect_pitch);
					blitter_colormode = zdata;
				}
				else if (zaddr==MNT_BASE_SCALEMODE) {
					vdiv=zdata;
				}

				else if (zaddr==MNT_BASE_MODE) {
					printf("mode change: %d\n",zdata);
					// https://github.com/Xilinx/embeddedsw/blob/master/XilinxProcessorIPLib/drivers/vtc/src/xvtc.c

					/*if (zdata==0) {
					    video_system_init(1280, 720, 1980, 750, 75, 60, hdiv, vdiv);
					} else if (zdata==1) {
					    video_system_init(800, 600, 1056, 628, 40, 60, hdiv, vdiv);
					} else if (zdata==2) {
					    video_system_init(640, 480, 800, 525, 25, 60, hdiv, vdiv);
					} else if (zdata==3) {
					    video_system_init(1024, 768, 1184, 806, 65, 60, hdiv, vdiv);
					} else if (zdata==4) {
					    video_system_init(1280, 1024, 1688, 1066, 108, 60, hdiv, vdiv);
					} else if (zdata==5) {
					    video_system_init(1920, 1080, 2640, 1125, 150, 50, hdiv, vdiv);
					} else if (zdata==6) {
					    video_system_init(720, 576, 864, 625, 27, 50, hdiv, vdiv);
					} else {
						printf("error: unknown mode\n");
					}*/
				}
				else if (zaddr==MNT_BASE_HSIZE) {
					//vmode_hsize=zdata;
					//init_vdma(vmode_hsize,vmode_vsize,hdiv,vdiv);
				}
				else if (zaddr==MNT_BASE_VSIZE) {
					//vmode_vsize=zdata;
					//init_vdma(vmode_hsize,vmode_vsize,hdiv,vdiv);
				}
				else if (zaddr==MNT_BASE_COLORMODE) {
					//colormode=zdata;
					//hdiv=zdata;
				}
				else if (zaddr==MNT_BASE_VIDEOCAP_MODE) {
					//videocap_mode=zdata;
				}
				else if (zaddr==MNT_BASE_ETH_TX) {
					ethernet_send_frame(zdata);
				}
				else if (zaddr==MNT_BASE_RUN_HI) {
					arm_run_address=((u32)zdata)<<16;
				}
				else if (zaddr==MNT_BASE_RUN_LO) {
					/*arm_run_address|=zdata;
					printf("[ARM_RUN] %lx\n",arm_run_address);
					void (*trampoline)(uint16_t x) = arm_run_address;
					trampoline(arm_run_arg0);*/
				}
				else if (zaddr==MNT_BASE_RUN_ARG0) {
					arm_run_arg0 = zdata;
				}
			}
			else if (zaddr>=MNT_FB_BASE) {
				u32 addr = zaddr-MNT_FB_BASE;
			    //addr ^= 2ul; // swap words (BE -> LE)
		        u32 z3 = (zstate_raw&(1<<25));

				if (z3) {
					if (ds3) mem[addr]   = zdata>>24;
					if (ds2) mem[addr+1] = zdata>>16;
					if (ds1) mem[addr+2] = zdata>>8;
					if (ds0) mem[addr+3] = zdata;
				} else {
					// swap bytes
					if (ds1) mem[addr]   = zdata>>8;
					if (ds0) mem[addr+1] = zdata;
				}
			}

			// ack the write
			MNTZORRO_mWriteReg(MNTZ_BASE_ADDR, MNTZORRO_S00_AXI_SLV_REG0_OFFSET, (1<<31));
			need_req_ack = 1;
		}
		else if (readreq) {
			uint32_t zaddr = MNTZORRO_mReadReg(MNTZ_BASE_ADDR, MNTZORRO_S00_AXI_SLV_REG0_OFFSET);
			//printf("READ: %08lx\n",zaddr);

			if (zaddr>0x10000000) {
				printf("ERRR: illegal address %08lx\n",zaddr);
			}
			else if (zaddr>=MNT_REG_BASE && zaddr<MNT_FB_BASE) {
				// read ARM "register"
				/*if (z3) {
					u32 b1 = mem[addr]<<24;
					u32 b2 = mem[addr+1]<<16;
					u32 b3 = mem[addr+2]<<8;
					u32 b4 = mem[addr+3];

					MNTZORRO_mWriteReg(MNTZ_BASE_ADDR, MNTZORRO_S00_AXI_SLV_REG1_OFFSET, b1|b2|b3|b4);
				}*/
				MNTZORRO_mWriteReg(MNTZ_BASE_ADDR, MNTZORRO_S00_AXI_SLV_REG1_OFFSET, 0);
			}
			else if (zaddr>=MNT_FB_BASE) {
				u32 addr = zaddr-MNT_FB_BASE;
			    //addr ^= 2ul; // swap words (BE -> LE)
		        u32 z3 = (zstate_raw&(1<<25)); // highly inefficient
				if (z3) {
					u32 b1 = mem[addr]<<24;
					u32 b2 = mem[addr+1]<<16;
					u32 b3 = mem[addr+2]<<8;
					u32 b4 = mem[addr+3];

					//printf("     '-> %08lx\n",b1|b2|b3|b4);

					MNTZORRO_mWriteReg(MNTZ_BASE_ADDR, MNTZORRO_S00_AXI_SLV_REG1_OFFSET, b1|b2|b3|b4);
				} else {
					u16 ubyte = mem[addr]<<8;
					u16 lbyte = mem[addr+1];
					MNTZORRO_mWriteReg(MNTZ_BASE_ADDR, MNTZORRO_S00_AXI_SLV_REG1_OFFSET, ubyte|lbyte);
				}
			}

			// ack the read
			MNTZORRO_mWriteReg(MNTZ_BASE_ADDR, MNTZORRO_S00_AXI_SLV_REG0_OFFSET, (1<<30));
			need_req_ack = 2;
		}
		else {
			// there are no read/write requests, we can do other housekeeping

			int interlace = !!(zstate_raw&(1<<24));
			if (interlace != interlace_old) {
				// interlace has changed, we need to reconfigure vdma for the new screen height
				if (interlace) {
					vdiv = 1;
				} else {
					vdiv = 2;
				}
				//init_vdma(vmode_hsize,vmode_vsize,hdiv,vdiv);
			}
			interlace_old = interlace;

			if (zstate==0) {
				// RESET
				hdiv = 1;
				vdiv = 2;
				handle_amiga_reset();
				/*usleep(10000);
	    		MNTZORRO_mWriteReg(MNTZ_BASE_ADDR, MNTZORRO_S00_AXI_SLV_REG3_OFFSET, 1);
	    		MNTZORRO_mWriteReg(MNTZ_BASE_ADDR, MNTZORRO_S00_AXI_SLV_REG2_OFFSET, 0xf0000000+0x5); // OP_VSYNC
	    		usleep(10000);
	    		MNTZORRO_mWriteReg(MNTZ_BASE_ADDR, MNTZORRO_S00_AXI_SLV_REG3_OFFSET, 0);
	    		MNTZORRO_mWriteReg(MNTZ_BASE_ADDR, MNTZORRO_S00_AXI_SLV_REG2_OFFSET, 0xf0); // NOP
	    		MNTZORRO_mWriteReg(MNTZ_BASE_ADDR, MNTZORRO_S00_AXI_SLV_REG2_OFFSET, 0); // NOP*/
			}
		}

		// TODO: potential hang, timeout?
        if (need_req_ack) {
        	while (1) {
        		u32 zstate = MNTZORRO_mReadReg(MNTZ_BASE_ADDR, MNTZORRO_S00_AXI_SLV_REG3_OFFSET);
                u32 writereq   = (zstate&(1<<31));
                u32 readreq    = (zstate&(1<<30));
                if (need_req_ack==1 && !writereq) break;
                if (need_req_ack==2 && !readreq) break;
                if ((zstate&0xff) == 0) break;
        	}
    		MNTZORRO_mWriteReg(MNTZ_BASE_ADDR, MNTZORRO_S00_AXI_SLV_REG0_OFFSET, 0);
    		need_req_ack = 0;
        }
    }

    cleanup_platform();
    return 0;
}
