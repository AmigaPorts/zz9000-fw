//Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2018.3 (lin64) Build 2405991 Thu Dec  6 23:36:41 MST 2018
//Date        : Tue Jan  8 15:24:37 2019
//Host        : doom running 64-bit Debian GNU/Linux buster/sid
//Command     : generate_target zz9000_ps_wrapper.bd
//Design      : zz9000_ps_wrapper
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

module zz9000_ps_wrapper
   (DDR_addr,
    DDR_ba,
    DDR_cas_n,
    DDR_ck_n,
    DDR_ck_p,
    DDR_cke,
    DDR_cs_n,
    DDR_dm,
    DDR_dq,
    DDR_dqs_n,
    DDR_dqs_p,
    DDR_odt,
    DDR_ras_n,
    DDR_reset_n,
    DDR_we_n,
    FIXED_IO_ddr_vrn,
    FIXED_IO_ddr_vrp,
    FIXED_IO_mio,
    FIXED_IO_ps_clk,
    FIXED_IO_ps_porb,
    FIXED_IO_ps_srstb,
    HDMI_INTN,
    I2C0_scl_io,
    I2C0_sda_io,
    VGA_B,
    VGA_DE,
    VGA_G,
    VGA_HS,
    VGA_PCLK,
    VGA_R,
    VGA_VS,
    ZORRO_ADDR,
    ZORRO_ADDRDIR,
    ZORRO_DATA,
    ZORRO_DATADIR,
    ZORRO_DOE,
    ZORRO_E7M,
    ZORRO_NCCS,
    ZORRO_NCFGIN,
    ZORRO_NCFGOUT,
    ZORRO_NCINH,
    ZORRO_NDS0,
    ZORRO_NDS1,
    ZORRO_NDTACK,
    ZORRO_NFCS,
    ZORRO_NIORST,
    ZORRO_NLDS,
    ZORRO_NMTCR,
    ZORRO_NSLAVE,
    ZORRO_NUDS,
    ZORRO_READ);
  inout [14:0]DDR_addr;
  inout [2:0]DDR_ba;
  inout DDR_cas_n;
  inout DDR_ck_n;
  inout DDR_ck_p;
  inout DDR_cke;
  inout DDR_cs_n;
  inout [3:0]DDR_dm;
  inout [31:0]DDR_dq;
  inout [3:0]DDR_dqs_n;
  inout [3:0]DDR_dqs_p;
  inout DDR_odt;
  inout DDR_ras_n;
  inout DDR_reset_n;
  inout DDR_we_n;
  inout FIXED_IO_ddr_vrn;
  inout FIXED_IO_ddr_vrp;
  inout [53:0]FIXED_IO_mio;
  inout FIXED_IO_ps_clk;
  inout FIXED_IO_ps_porb;
  inout FIXED_IO_ps_srstb;
  inout HDMI_INTN;
  inout I2C0_scl_io;
  inout I2C0_sda_io;
  output [4:0]VGA_B;
  output VGA_DE;
  output [5:0]VGA_G;
  output VGA_HS;
  output VGA_PCLK;
  output [4:0]VGA_R;
  output VGA_VS;
  inout [22:0]ZORRO_ADDR;
  output ZORRO_ADDRDIR;
  inout [15:0]ZORRO_DATA;
  output ZORRO_DATADIR;
  input ZORRO_DOE;
  input ZORRO_E7M;
  input ZORRO_NCCS;
  input ZORRO_NCFGIN;
  output ZORRO_NCFGOUT;
  output ZORRO_NCINH;
  input ZORRO_NDS0;
  input ZORRO_NDS1;
  output ZORRO_NDTACK;
  input ZORRO_NFCS;
  input ZORRO_NIORST;
  input ZORRO_NLDS;
  input ZORRO_NMTCR;
  output ZORRO_NSLAVE;
  input ZORRO_NUDS;
  input ZORRO_READ;

  wire [14:0]DDR_addr;
  wire [2:0]DDR_ba;
  wire DDR_cas_n;
  wire DDR_ck_n;
  wire DDR_ck_p;
  wire DDR_cke;
  wire DDR_cs_n;
  wire [3:0]DDR_dm;
  wire [31:0]DDR_dq;
  wire [3:0]DDR_dqs_n;
  wire [3:0]DDR_dqs_p;
  wire DDR_odt;
  wire DDR_ras_n;
  wire DDR_reset_n;
  wire DDR_we_n;
  wire FIXED_IO_ddr_vrn;
  wire FIXED_IO_ddr_vrp;
  wire [53:0]FIXED_IO_mio;
  wire FIXED_IO_ps_clk;
  wire FIXED_IO_ps_porb;
  wire FIXED_IO_ps_srstb;
  wire HDMI_INTN;
  wire I2C0_scl_i;
  wire I2C0_scl_io;
  wire I2C0_scl_o;
  wire I2C0_scl_t;
  wire I2C0_sda_i;
  wire I2C0_sda_io;
  wire I2C0_sda_o;
  wire I2C0_sda_t;
  wire [4:0]VGA_B;
  wire VGA_DE;
  wire [5:0]VGA_G;
  wire VGA_HS;
  wire VGA_PCLK;
  wire [4:0]VGA_R;
  wire VGA_VS;
  wire [22:0]ZORRO_ADDR;
  wire ZORRO_ADDRDIR;
  wire [15:0]ZORRO_DATA;
  wire ZORRO_DATADIR;
  wire ZORRO_DOE;
  wire ZORRO_E7M;
  wire ZORRO_NCCS;
  wire ZORRO_NCFGIN;
  wire ZORRO_NCFGOUT;
  wire ZORRO_NCINH;
  wire ZORRO_NDS0;
  wire ZORRO_NDS1;
  wire ZORRO_NDTACK;
  wire ZORRO_NFCS;
  wire ZORRO_NIORST;
  wire ZORRO_NLDS;
  wire ZORRO_NMTCR;
  wire ZORRO_NSLAVE;
  wire ZORRO_NUDS;
  wire ZORRO_READ;

  IOBUF I2C0_scl_iobuf
       (.I(I2C0_scl_o),
        .IO(I2C0_scl_io),
        .O(I2C0_scl_i),
        .T(I2C0_scl_t));
  IOBUF I2C0_sda_iobuf
       (.I(I2C0_sda_o),
        .IO(I2C0_sda_io),
        .O(I2C0_sda_i),
        .T(I2C0_sda_t));
  zz9000_ps zz9000_ps_i
       (.DDR_addr(DDR_addr),
        .DDR_ba(DDR_ba),
        .DDR_cas_n(DDR_cas_n),
        .DDR_ck_n(DDR_ck_n),
        .DDR_ck_p(DDR_ck_p),
        .DDR_cke(DDR_cke),
        .DDR_cs_n(DDR_cs_n),
        .DDR_dm(DDR_dm),
        .DDR_dq(DDR_dq),
        .DDR_dqs_n(DDR_dqs_n),
        .DDR_dqs_p(DDR_dqs_p),
        .DDR_odt(DDR_odt),
        .DDR_ras_n(DDR_ras_n),
        .DDR_reset_n(DDR_reset_n),
        .DDR_we_n(DDR_we_n),
        .FIXED_IO_ddr_vrn(FIXED_IO_ddr_vrn),
        .FIXED_IO_ddr_vrp(FIXED_IO_ddr_vrp),
        .FIXED_IO_mio(FIXED_IO_mio),
        .FIXED_IO_ps_clk(FIXED_IO_ps_clk),
        .FIXED_IO_ps_porb(FIXED_IO_ps_porb),
        .FIXED_IO_ps_srstb(FIXED_IO_ps_srstb),
        .HDMI_INTN(HDMI_INTN),
        .I2C0_scl_i(I2C0_scl_i),
        .I2C0_scl_o(I2C0_scl_o),
        .I2C0_scl_t(I2C0_scl_t),
        .I2C0_sda_i(I2C0_sda_i),
        .I2C0_sda_o(I2C0_sda_o),
        .I2C0_sda_t(I2C0_sda_t),
        .VGA_B(VGA_B),
        .VGA_DE(VGA_DE),
        .VGA_G(VGA_G),
        .VGA_HS(VGA_HS),
        .VGA_PCLK(VGA_PCLK),
        .VGA_R(VGA_R),
        .VGA_VS(VGA_VS),
        .ZORRO_ADDR(ZORRO_ADDR),
        .ZORRO_ADDRDIR(ZORRO_ADDRDIR),
        .ZORRO_DATA(ZORRO_DATA),
        .ZORRO_DATADIR(ZORRO_DATADIR),
        .ZORRO_DOE(ZORRO_DOE),
        .ZORRO_E7M(ZORRO_E7M),
        .ZORRO_NCCS(ZORRO_NCCS),
        .ZORRO_NCFGIN(ZORRO_NCFGIN),
        .ZORRO_NCFGOUT(ZORRO_NCFGOUT),
        .ZORRO_NCINH(ZORRO_NCINH),
        .ZORRO_NDS0(ZORRO_NDS0),
        .ZORRO_NDS1(ZORRO_NDS1),
        .ZORRO_NDTACK(ZORRO_NDTACK),
        .ZORRO_NFCS(ZORRO_NFCS),
        .ZORRO_NIORST(ZORRO_NIORST),
        .ZORRO_NLDS(ZORRO_NLDS),
        .ZORRO_NMTCR(ZORRO_NMTCR),
        .ZORRO_NSLAVE(ZORRO_NSLAVE),
        .ZORRO_NUDS(ZORRO_NUDS),
        .ZORRO_READ(ZORRO_READ));
endmodule
