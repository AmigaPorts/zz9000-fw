`timescale 1 ns / 1 ps

`define C_S_AXI_DATA_WIDTH 32
`define C_S_AXI_ADDR_WIDTH 4
`define RAM_SIZE 32'h400000
`define REG_SIZE 32'h01000
`define AUTOCONF_LOW  24'he80000
`define AUTOCONF_HIGH 24'he80080
`define Z3_RAM_SIZE 32'h10000000 // 256MB
`define ARM_MEMORY_START 32'h100000

`define C_M00_AXI_TARGET_SLAVE_BASE_ADDR 32'h10000000
`define C_M00_AXI_ID_WIDTH   1
`define C_M00_AXI_ADDR_WIDTH 32
`define C_M00_AXI_DATA_WIDTH 32
`define C_M00_AXI_AWUSER_WIDTH 0
`define C_M00_AXI_ARUSER_WIDTH 0
`define C_M00_AXI_WUSER_WIDTH	0
`define C_M00_AXI_RUSER_WIDTH	0
`define C_M00_AXI_BUSER_WIDTH	0

module MNTZorro_v0_1_S00_AXI
(
  inout wire [22:0] ZORRO_ADDR,
  inout wire [15:0] ZORRO_DATA,
  
  output wire ZORRO_DATADIR,
  output wire ZORRO_ADDRDIR,
  output wire ZORRO_ADDRDIR2,
  output wire ZORRO_NBRN,
  input  wire ZORRO_NBGN,
  
  input wire ZORRO_READ,
  //input wire ZORRO_NMTCR,
  input wire ZORRO_NUDS,
  input wire ZORRO_NLDS,
  input wire ZORRO_NDS1,
  input wire ZORRO_NDS0,
  input wire ZORRO_NCCS,
  input wire ZORRO_NFCS,
  input wire ZORRO_DOE,
  input wire ZORRO_NIORST,
  input wire ZORRO_NCFGIN,
  input wire ZORRO_E7M,
  input wire ZORRO_C28D,
  
  input wire VCAP_VSYNC,
  input wire VCAP_HSYNC,
  input wire VCAP_G0,
  input wire VCAP_G1,
  input wire VCAP_G2,
  input wire VCAP_G3,
  input wire VCAP_G4,
  input wire VCAP_G5,
  input wire VCAP_G6,
  input wire VCAP_G7,
  
  input wire VCAP_B7,
  input wire VCAP_B6,
  input wire VCAP_B5,
  input wire VCAP_B4,
  input wire VCAP_B3,
  input wire VCAP_B2,
  input wire VCAP_B1,
  input wire VCAP_B0,
  
  input wire VCAP_R7,
  input wire VCAP_R6,
  input wire VCAP_R5,
  input wire VCAP_R4,
  input wire VCAP_R3,
  input wire VCAP_R2,
  input wire VCAP_R1,
  input wire VCAP_R0,
  
  output wire ZORRO_NCFGOUT,
  output wire ZORRO_NSLAVE,
  output wire ZORRO_NCINH,
  output wire ZORRO_NDTACK,
	
	//  HP master interface to write to PS memory directly
	input wire m00_axi_aclk,
	input wire m00_axi_aresetn,
  // write address channel
  input wire m00_axi_awready,
  //output reg [C_M00_AXI_ID_WIDTH-1 : 0] m00_axi_awid,
  output reg [`C_M00_AXI_ADDR_WIDTH-1 : 0] m00_axi_awaddr,
  output reg [7:0] m00_axi_awlen,
  output reg [2:0] m00_axi_awsize,
  output reg [1:0] m00_axi_awburst,
  output reg m00_axi_awlock,
  output reg [3:0] m00_axi_awcache,
  output reg [2:0] m00_axi_awprot,
  output reg [3:0] m00_axi_awqos,
  output reg m00_axi_awvalid,
  
  // write channel
  input wire m00_axi_wready,
  //output reg [C_M00_AXI_ID_WIDTH-1 : 0] m00_axi_wid,
  output reg [`C_M00_AXI_DATA_WIDTH-1 : 0] m00_axi_wdata,
  output reg [`C_M00_AXI_DATA_WIDTH/8-1 : 0] m00_axi_wstrb,
  output reg m00_axi_wlast,
  output reg m00_axi_wvalid,
  
  // buffered write response channel
  //input wire [C_M00_AXI_ID_WIDTH-1 : 0] m00_axi_bid,
  input wire [1 : 0] m00_axi_bresp,
  input wire m00_axi_bvalid,
  output reg m00_axi_bready,
  
  // read address channel
  /*input wire m00_axi_arready,
  output reg [C_M00_AXI_ID_WIDTH-1 : 0] m00_axi_arid,
  output reg [C_M00_AXI_ADDR_WIDTH-1 : 0] m00_axi_araddr,
  output reg [7 : 0] m00_axi_arlen,
  output reg [2 : 0] m00_axi_arsize,
  output reg [1 : 0] m00_axi_arburst,
  output reg m00_axi_arlock,
  output reg [3 : 0] m00_axi_arcache,
  output reg [2 : 0] m00_axi_arprot,
  output reg [3 : 0] m00_axi_arqos,
  output reg m00_axi_arvalid,
  
  output reg m00_axi_rready,
  input wire [C_M00_AXI_ID_WIDTH-1 : 0] m00_axi_rid,
  input wire [C_M00_AXI_DATA_WIDTH-1 : 0] m00_axi_rdata,
  input wire [1 : 0] m00_axi_rresp,
  input wire m00_axi_rlast,
  input wire m00_axi_rvalid,*/
  
  output reg [31:0] video_control_data,
  output reg [7:0]  video_control_op,
  output reg video_control_interlace, 
  
	 // User ports ends
	 // Do not modify the ports beyond this line

	 // Global Clock Signal
	 input wire  S_AXI_ACLK,
	 // Global Reset Signal. This Signal is Active LOW
	 input wire  S_AXI_ARESETN,
	 // Write address (issued by master, acceped by Slave)
	 input wire [`C_S_AXI_ADDR_WIDTH-1 : 0] S_AXI_AWADDR,
	 // Write channel Protection type. This signal indicates the
   // privilege and security level of the transaction, and whether
   // the transaction is a data access or an instruction access.
	 input wire [2 : 0] S_AXI_AWPROT,
	 // Write address valid. This signal indicates that the master signaling
   // valid write address and control information.
	 input wire  S_AXI_AWVALID,
	 // Write address ready. This signal indicates that the slave is ready
   // to accept an address and associated control signals.
	 output wire  S_AXI_AWREADY,
	 // Write data (issued by master, acceped by Slave) 
	 input wire [`C_S_AXI_DATA_WIDTH-1 : 0] S_AXI_WDATA,
	 // Write strobes. This signal indicates which byte lanes hold
   // valid data. There is one write strobe bit for each eight
   // bits of the write data bus.    
	 input wire [(`C_S_AXI_DATA_WIDTH/8)-1 : 0] S_AXI_WSTRB,
	 // Write valid. This signal indicates that valid write
   // data and strobes are available.
	 input wire  S_AXI_WVALID,
	 // Write ready. This signal indicates that the slave
   // can accept the write data.
	 output wire  S_AXI_WREADY,
	 // Write response. This signal indicates the status
   // of the write transaction.
	 output wire [1 : 0] S_AXI_BRESP,
	 // Write response valid. This signal indicates that the channel
   // is signaling a valid write response.
	 output wire  S_AXI_BVALID,
	 // Response ready. This signal indicates that the master
   // can accept a write response.
	 input wire  S_AXI_BREADY,
	 // Read address (issued by master, acceped by Slave)
	 input wire [`C_S_AXI_ADDR_WIDTH-1 : 0] S_AXI_ARADDR,
	 // Protection type. This signal indicates the privilege
   // and security level of the transaction, and whether the
   // transaction is a data access or an instruction access.
	 input wire [2 : 0] S_AXI_ARPROT,
	 // Read address valid. This signal indicates that the channel
   // is signaling valid read address and control information.
	 input wire  S_AXI_ARVALID,
	 // Read address ready. This signal indicates that the slave is
   // ready to accept an address and associated control signals.
	 output wire  S_AXI_ARREADY,
	 // Read data (issued by slave)
	 output wire [`C_S_AXI_DATA_WIDTH-1 : 0] S_AXI_RDATA,
	 // Read response. This signal indicates the status of the
   // read transfer.
	 output wire [1 : 0] S_AXI_RRESP,
	 // Read valid. This signal indicates that the channel is
   // signaling the required read data.
	 output wire  S_AXI_RVALID,
	 // Read ready. This signal indicates that the master can
   // accept the read data and response information.
	 input wire  S_AXI_RREADY
	 );

	// AXI4LITE signals
	reg [`C_S_AXI_ADDR_WIDTH-1 : 0] 	axi_awaddr;
	reg  	axi_awready;
	reg  	axi_wready;
	reg [1 : 0] 	axi_bresp;
	reg  	axi_bvalid;
	reg [`C_S_AXI_ADDR_WIDTH-1 : 0] 	axi_araddr;
	reg  	axi_arready;
	reg [`C_S_AXI_DATA_WIDTH-1 : 0] 	axi_rdata;
	reg [1 : 0] 	axi_rresp;
	reg  	axi_rvalid;

	// Example-specific design signals
	// local localparam for addressing 32 bit / 64 bit C_S_AXI_DATA_WIDTH
	// ADDR_LSB is used for addressing 32/64 bit registers/memories
	// ADDR_LSB = 2 for 32 bits (n downto 2)
	// ADDR_LSB = 3 for 64 bits (n downto 3)
	localparam integer ADDR_LSB = (`C_S_AXI_DATA_WIDTH/32) + 1;
	localparam integer OPT_MEM_ADDR_BITS = 1;
	//----------------------------------------------
	//-- Signals for user logic register space example
	//------------------------------------------------
	//-- Number of Slave Registers 4
	reg [`C_S_AXI_DATA_WIDTH-1:0]	slv_reg0;
	reg [`C_S_AXI_DATA_WIDTH-1:0]	slv_reg1;
	reg [`C_S_AXI_DATA_WIDTH-1:0]	slv_reg2;
	reg [`C_S_AXI_DATA_WIDTH-1:0]	slv_reg3;
	wire	 slv_reg_rden;
	wire	 slv_reg_wren;
	reg [`C_S_AXI_DATA_WIDTH-1:0]	 reg_data_out;
	integer	 byte_index;
	reg	 aw_en;
	
	reg [`C_S_AXI_DATA_WIDTH-1:0] out_reg0;
	reg [`C_S_AXI_DATA_WIDTH-1:0] out_reg1;
	reg [`C_S_AXI_DATA_WIDTH-1:0] out_reg2;
	reg [`C_S_AXI_DATA_WIDTH-1:0] out_reg3;	

	// I/O Connections assignments

	assign S_AXI_AWREADY	= axi_awready;
	assign S_AXI_WREADY	= axi_wready;
	assign S_AXI_BRESP	= axi_bresp;
	assign S_AXI_BVALID	= axi_bvalid;
	assign S_AXI_ARREADY	= axi_arready;
	assign S_AXI_RDATA	= axi_rdata;
	assign S_AXI_RRESP	= axi_rresp;
	assign S_AXI_RVALID	= axi_rvalid;
	// Implement axi_awready generation
	// axi_awready is asserted for one S_AXI_ACLK clock cycle when both
	// S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_awready is
	// de-asserted when reset is low.

	always @( posedge S_AXI_ACLK )
	  begin
	    if ( S_AXI_ARESETN == 1'b0 )
	      begin
	        axi_awready <= 1'b0;
	        aw_en <= 1'b1;
	      end 
	    else
	      begin    
	        if (~axi_awready && S_AXI_AWVALID && S_AXI_WVALID && aw_en)
	          begin
	            // slave is ready to accept write address when 
	            // there is a valid write address and write data
	            // on the write address and data bus. This design 
	            // expects no outstanding transactions. 
	            axi_awready <= 1'b1;
	            aw_en <= 1'b0;
	          end
	        else if (S_AXI_BREADY && axi_bvalid)
	          begin
	            aw_en <= 1'b1;
	            axi_awready <= 1'b0;
	          end
	             else           
	               begin
	                 axi_awready <= 1'b0;
	               end
	      end 
	  end       

	// Implement axi_awaddr latching
	// This process is used to latch the address when both 
	// S_AXI_AWVALID and S_AXI_WVALID are valid. 

	always @( posedge S_AXI_ACLK )
	  begin
	    if ( S_AXI_ARESETN == 1'b0 )
	      begin
	        axi_awaddr <= 0;
	      end 
	    else
	      begin    
	        if (~axi_awready && S_AXI_AWVALID && S_AXI_WVALID && aw_en)
	          begin
	            // Write Address latching 
	            axi_awaddr <= S_AXI_AWADDR;
	          end
	      end 
	  end       

	// Implement axi_wready generation
	// axi_wready is asserted for one S_AXI_ACLK clock cycle when both
	// S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_wready is 
	// de-asserted when reset is low. 

	always @( posedge S_AXI_ACLK )
	  begin
	    if ( S_AXI_ARESETN == 1'b0 )
	      begin
	        axi_wready <= 1'b0;
	      end 
	    else
	      begin    
	        if (~axi_wready && S_AXI_WVALID && S_AXI_AWVALID && aw_en )
	          begin
	            // slave is ready to accept write data when 
	            // there is a valid write address and write data
	            // on the write address and data bus. This design 
	            // expects no outstanding transactions. 
	            axi_wready <= 1'b1;
	          end
	        else
	          begin
	            axi_wready <= 1'b0;
	          end
	      end 
	  end       

	// Implement memory mapped register select and write logic generation
	// The write data is accepted and written to memory mapped registers when
	// axi_awready, S_AXI_WVALID, axi_wready and S_AXI_WVALID are asserted. Write strobes are used to
	// select byte enables of slave registers while writing.
	// These registers are cleared when reset (active low) is applied.
	// Slave register write enable is asserted when valid address and data are available
	// and the slave is ready to accept the write address and write data.
	assign slv_reg_wren = axi_wready && S_AXI_WVALID && axi_awready && S_AXI_AWVALID;

	always @( posedge S_AXI_ACLK )
	  begin
	    if ( S_AXI_ARESETN == 1'b0 )
	      begin
	        slv_reg0 <= 0;
	        slv_reg1 <= 0;
	        slv_reg2 <= 0;
	        slv_reg3 <= 0;
	      end 
	    else begin
	      if (slv_reg_wren)
	        begin
	          case ( axi_awaddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] )
	            2'h0:
	              for ( byte_index = 0; byte_index <= (`C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	                if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                  // Respective byte enables are asserted as per write strobes 
	                  // Slave register 0
	                  slv_reg0[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	                end  
	            2'h1:
	              for ( byte_index = 0; byte_index <= (`C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	                if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                  // Respective byte enables are asserted as per write strobes 
	                  // Slave register 1
	                  slv_reg1[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	                end  
	            2'h2:
	              for ( byte_index = 0; byte_index <= (`C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	                if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                  // Respective byte enables are asserted as per write strobes 
	                  // Slave register 2
	                  slv_reg2[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	                end  
	            2'h3:
	              for ( byte_index = 0; byte_index <= (`C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	                if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                  // Respective byte enables are asserted as per write strobes 
	                  // Slave register 3
	                  slv_reg3[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	                end  
	            default : begin
	              slv_reg0 <= slv_reg0;
	              slv_reg1 <= slv_reg1;
	              slv_reg2 <= slv_reg2;
	              slv_reg3 <= slv_reg3;
	            end
	          endcase
	        end
	    end
	  end    

	// Implement write response logic generation
	// The write response and response valid signals are asserted by the slave 
	// when axi_wready, S_AXI_WVALID, axi_wready and S_AXI_WVALID are asserted.  
	// This marks the acceptance of address and indicates the status of 
	// write transaction.

	always @( posedge S_AXI_ACLK )
	  begin
	    if ( S_AXI_ARESETN == 1'b0 )
	      begin
	        axi_bvalid  <= 0;
	        axi_bresp   <= 2'b0;
	      end 
	    else
	      begin    
	        if (axi_awready && S_AXI_AWVALID && ~axi_bvalid && axi_wready && S_AXI_WVALID)
	          begin
	            // indicates a valid write response is available
	            axi_bvalid <= 1'b1;
	            axi_bresp  <= 2'b0; // 'OKAY' response 
	          end                   // work error responses in future
	        else
	          begin
	            if (S_AXI_BREADY && axi_bvalid) 
	              //check if bready is asserted while bvalid is high) 
	              //(there is a possibility that bready is always asserted high)   
	              begin
	                axi_bvalid <= 1'b0; 
	              end  
	          end
	      end
	  end   

	// Implement axi_arready generation
	// axi_arready is asserted for one S_AXI_ACLK clock cycle when
	// S_AXI_ARVALID is asserted. axi_awready is 
	// de-asserted when reset (active low) is asserted. 
	// The read address is also latched when S_AXI_ARVALID is 
	// asserted. axi_araddr is reset to zero on reset assertion.

	always @( posedge S_AXI_ACLK )
	  begin
	    if ( S_AXI_ARESETN == 1'b0 )
	      begin
	        axi_arready <= 1'b0;
	        axi_araddr  <= 32'b0;
	      end 
	    else
	      begin    
	        if (~axi_arready && S_AXI_ARVALID)
	          begin
	            // indicates that the slave has acceped the valid read address
	            axi_arready <= 1'b1;
	            // Read address latching
	            axi_araddr  <= S_AXI_ARADDR;
	          end
	        else
	          begin
	            axi_arready <= 1'b0;
	          end
	      end 
	  end       

	// Implement axi_arvalid generation
	// axi_rvalid is asserted for one S_AXI_ACLK clock cycle when both 
	// S_AXI_ARVALID and axi_arready are asserted. The slave registers 
	// data are available on the axi_rdata bus at this instance. The 
	// assertion of axi_rvalid marks the validity of read data on the 
	// bus and axi_rresp indicates the status of read transaction.axi_rvalid 
	// is deasserted on reset (active low). axi_rresp and axi_rdata are 
	// cleared to zero on reset (active low).  
	always @( posedge S_AXI_ACLK )
	  begin
	    if ( S_AXI_ARESETN == 1'b0 )
	      begin
	        axi_rvalid <= 0;
	        axi_rresp  <= 0;
	      end 
	    else
	      begin    
	        if (axi_arready && S_AXI_ARVALID && ~axi_rvalid)
	          begin
	            // Valid read data is available at the read data bus
	            axi_rvalid <= 1'b1;
	            axi_rresp  <= 2'b0; // 'OKAY' response
	          end   
	        else if (axi_rvalid && S_AXI_RREADY)
	          begin
	            // Read data is accepted by the master
	            axi_rvalid <= 1'b0;
	          end                
	      end
	  end    

	// Output register or memory read data
	always @( posedge S_AXI_ACLK )
	  begin
	    if ( S_AXI_ARESETN == 1'b0 )
	      begin
	        axi_rdata  <= 0;
	      end 
	    else
	      begin    
	        // When there is a valid read address (S_AXI_ARVALID) with 
	        // acceptance of read address by the slave (axi_arready), 
	        // output the read dada 
	        if (slv_reg_rden)
	          begin
	            axi_rdata <= reg_data_out;     // register read data
	          end
	      end
	  end

  
  reg [3:0] znAS_sync  = 3'b111;
  reg [3:0] znUDS_sync = 3'b000;
  reg [3:0] znLDS_sync = 3'b000;
  reg [3:0] zREAD_sync = 3'b000;
  
  reg [4:0] znFCS_sync = 5'b11111;
  reg [2:0] znDS1_sync = 3'b000;
  reg [2:0] znDS0_sync = 3'b000;
  reg [1:0] znRST_sync = 2'b11;
  reg [1:0] zDOE_sync = 2'b00;
  reg [4:0] zE7M_sync = 5'b00000;
  reg [2:0] znCFGIN_sync = 3'b000;
  
  reg [23:0] zaddr; // zorro 2 address
  reg [23:0] zaddr_sync;
  reg [23:0] zaddr_sync2;
  reg [15:0] zdata_in_sync;
  reg z2_addr_valid = 0;
  reg [23:0] z2_mapped_addr;
  reg z2_read = 0;
  reg z2_write = 0;
  reg datastrobe_synced = 0;
  reg zaddr_in_ram = 0;
  reg zaddr_in_reg = 0;
  reg zaddr_autoconfig = 0;
  reg [31:0] ram_low = 32'h610000;
  reg [31:0] ram_high = 32'ha00000;
  reg [31:0] reg_low  = 32'h601000;
  reg [31:0] reg_high = 32'h602000;
  reg z2_uds = 0;
  reg z2_lds = 0;

  reg [31:0] z3_ram_low = 32'h50000000;
  reg [31:0] z3_ram_high = 32'h50000000 + `Z3_RAM_SIZE -4;
  reg [31:0] z3_reg_low  = 32'h50001000;
  reg [31:0] z3_reg_high = 32'h50002000;
  reg [15:0] data_z3_hi16;
  reg [15:0] data_z3_low16;
  
  reg [15:0] data_z3_hi16_latched;
  reg [15:0] data_z3_low16_latched;
  
  reg [15:0] data_in_z3_low16;
  reg [15:0] z3_din_high_s2;
  reg [15:0] z3_din_low_s2;
  reg [31:0] z3addr;
  reg [31:0] last_z3addr;
  reg [31:0] z3addr2;
  reg [31:0] z3addr3;
  reg [31:0] z3_mapped_addr;
  reg [31:0] z3_read_addr;
  reg [15:0] z3_read_data;
  reg z3_din_latch = 0;
  reg z3_fcs_state = 1;
  reg z3_end_cycle = 0;
  
  reg z3addr_in_ram = 0;
  reg z3addr_in_reg = 0;
  reg z3addr_autoconfig = 0;

  reg ZORRO3 = 1;
  reg dataout = 0;
  reg dataout_z3 = 0;
  reg dataout_enable = 0;
  reg slaven = 0;
  reg dtack = 0;
  reg dtack_latched = 0;

  reg z_reset = 0;
  reg z_cfgin = 0;
  reg z_cfgin_lo = 0;
  reg z3_confdone = 0;

  reg zorro_read = 0;
  reg zorro_write = 0;
  
  reg [15:0] data_in;
  reg [31:0] rr_data;
  reg [15:0] data_out;
  reg [15:0] regdata_in;

  // ram arbiter
  reg zorro_ram_read_request = 0;
  reg zorro_ram_write_request = 0;
  reg [31:0] zorro_ram_read_addr;
  reg [31:0] zorro_ram_read_data;
  reg [3:0] zorro_ram_read_bytes;
  reg [31:0] zorro_ram_write_addr;
  reg [31:0] zorro_ram_write_data;
  reg [3:0] zorro_ram_write_bytes;

  reg [15:0] default_data = 'hffff; // causes read/write glitches on A2000 (data bus interference) when 0
  reg [1:0] zorro_write_capture_bytes = 0;
  reg [15:0] zorro_write_capture_data = 0;

  // z3 strobes
  reg z3_ds3=0;
  reg z3_ds2=0;
  reg z3_ds1=0;
  reg z3_ds0=0;
  
  // level shifter direction pins
  assign ZORRO_DATADIR     = ZORRO_DOE & (dataout_enable | dataout_z3); // d2-d9  d10-15, d0-d1
  assign ZORRO_ADDRDIR     = ZORRO_DOE & (dataout_z3); // a16-a23 <- input  a8-a15 <- input
  assign ZORRO_ADDRDIR2    = 0; //ZORRO_DOE & (dataout_z3_latched);
  assign ZORRO_NBRN = 1; // TODO busmastering
  
  wire ZORRO_DATA_T = ~(ZORRO_DOE & (dataout_enable | dataout_z3));
  wire ZORRO_ADDR_T = ~(ZORRO_DOE & dataout_z3);
  
  reg z_ovr = 0;
  assign ZORRO_NCINH = z_ovr?1'b1:1'b0; // inverse
  
  assign ZORRO_NSLAVE = (slaven)?1'b0:1'b1;
  // FIXME do we need to gate NDTACK like this?
  assign ZORRO_NDTACK  = (ZORRO_DOE & dtack_latched)?1'b1:1'b0; // inverse
  wire [22:0] z3_addr_out = {data_z3_low16_latched, 7'bZZZ_ZZZZ}; // FIXME this creates tri-cell warning?
  //wire [22:0] z3_addr_out = {data_z3_low16_latched, 7'b111_1111}; // FIXME this creates tri-cell warning?
  
  wire [15:0] ZORRO_DATA_IN;
  wire [22:0] ZORRO_ADDR_IN;
  
  //assign ZORRO_ADDR_IN = ZORRO_ADDR;
  //assign ZORRO_ADDR = (ZORRO_ADDR_T ? z3_addr_out : 23'bZZZ_ZZZZ_ZZZZ_ZZZZ_ZZZZ_ZZZZ);
  
  genvar i;
  
  generate
      for (i=0; i < 16; i=i+1) begin : ZORRO_DATABUS
          IOBUF u_iobuf_dq 
          (
              .I  (ZORRO3 ? data_z3_hi16_latched[i] : data_out[i]),
              .T  (ZORRO_DATA_T),
              .IO (ZORRO_DATA[i]),
              .O  (ZORRO_DATA_IN[i])
          );
      end
  endgenerate
    
    generate
        for (i=0; i < 23; i=i+1) begin : ZORRO_ADDRBUS
            IOBUF u_iobuf_dq 
            (
                .I  (z3_addr_out[i]),
                .T  (ZORRO_ADDR_T),
                .IO (ZORRO_ADDR[i]),
                .O  (ZORRO_ADDR_IN[i])
            );
        end
    endgenerate

  // autoconf output signal

  reg z_confout = 0;
  assign ZORRO_NCFGOUT = ZORRO_NCFGIN?1'b1:(~z_confout);
  
  reg [7:0] video_debug_reg;

  // -- synchronizers ------------------------------------------
  always @(posedge S_AXI_ACLK) begin
    znUDS_sync  <= {znUDS_sync[1:0],ZORRO_NUDS};
    znLDS_sync  <= {znLDS_sync[1:0],ZORRO_NLDS};
    znAS_sync   <= {znAS_sync[1:0],ZORRO_NCCS};
    zREAD_sync  <= {zREAD_sync[1:0],ZORRO_READ};
    
    znDS1_sync  <= {znDS1_sync[1:0],ZORRO_NDS1};
    znDS0_sync  <= {znDS0_sync[1:0],ZORRO_NDS0};
    znFCS_sync  <= {znFCS_sync[3:0],ZORRO_NFCS};
    znCFGIN_sync<= {znCFGIN_sync[1:0],ZORRO_NCFGIN};
    zDOE_sync   <= {zDOE_sync[0],ZORRO_DOE};
    
    znRST_sync  <= {znRST_sync[0],ZORRO_NIORST};
    
    // Z2 ------------------------------------------------
    z2_addr_valid <= (znAS_sync[2]==0 && znAS_sync[1]==0);
    
    zdata_in_sync <= ZORRO_DATA_IN;
    
    zaddr <= ZORRO_ADDR_IN[22:0];
    zaddr_sync  <= zaddr;
    zaddr_sync2 <= zaddr_sync;
    
    z2_mapped_addr <= {zaddr_sync2[22:0],1'b0};
    z2_read  <= (zREAD_sync[0] == 1'b1);
    z2_write <= (zREAD_sync[0] == 1'b0);
    
    datastrobe_synced <= (znUDS_sync==0 || znLDS_sync==0);
    z2_uds <= (znUDS_sync==0);
    z2_lds <= (znLDS_sync==0);
    
    zaddr_in_ram <= (z2_mapped_addr>=ram_low && z2_mapped_addr<ram_high);
    zaddr_in_reg <= (z2_mapped_addr>=reg_low && z2_mapped_addr<reg_high);
    
    if (znAS_sync[1]==0 && z2_mapped_addr>=`AUTOCONF_LOW && z2_mapped_addr<`AUTOCONF_HIGH)
      zaddr_autoconfig <= 1'b1;
    else
      zaddr_autoconfig <= 1'b0;
    
    // Z3 ------------------------------------------------
    z3addr2 <= {ZORRO_DATA_IN[15:8],ZORRO_ADDR_IN[22:1],2'b00};
    //z3addr2 <= {zD[15:8],zaddr[23:2],2'b00};
    
    // sample z3addr on falling edge of /FCS
    // A4000 needs [0] here, [1] worked for A3000
    case (znFCS_sync[1])
      1'b1: begin
        // these guards are here so that the values are only sampled
        // on the edges of this signal
        if (z3_fcs_state == 0) begin
          z3_fcs_state <= 1;
          z3_end_cycle <= 1;
          z3addr <= 0;
        end
      end
      1'b0: begin
        // CHECK: if responding too quickly, this causes crashes
        if (z3_fcs_state == 1) begin
          z3_fcs_state <= 0;
          z3_end_cycle <= 0;
          z3addr <= z3addr2;
        end
      end
    endcase
    
    zorro_read  <= zREAD_sync[0];
    zorro_write <= ~zREAD_sync[0];
    
    z3addr_in_ram <= (z3addr >= z3_ram_low) && (z3addr < z3_ram_high);
    z3addr_in_reg <= (z3addr >= z3_reg_low) && (z3addr < z3_reg_high);

    z3addr_autoconfig <= (z3addr[31:16]=='hff00);
    
    z3_mapped_addr <= (z3addr-z3_ram_low);
    data_in_z3_low16 <= ZORRO_ADDR_IN[22:7]; //zA[22:7]; // FIXME why sample this twice?
    
    if (znUDS_sync[1]==0 || znLDS_sync[1]==0 || znDS1_sync[1]==0 || znDS0_sync[1]==0)
      z3_din_latch <= 1;
    else
      z3_din_latch <= 0;
    
    // pipelined for better timing
    if (z3_din_latch) begin
      z3_din_high_s2 <= zdata_in_sync; //zD;
      z3_din_low_s2  <= data_in_z3_low16; //zA[22:7];
    end
    
    // pipelined for better timing
    data_z3_hi16_latched <= data_z3_hi16;
    data_z3_low16_latched <= data_z3_low16;
    
    dtack_latched <= dtack;
    
    z_reset <= (znRST_sync==2'b00);
    z_cfgin <= (znCFGIN_sync==3'b000);
    z_cfgin_lo <= (znCFGIN_sync==3'b111);
    
    //video_debug_reg <= video_debug;
  end // always @ (posedge S_AXI_ACLK)

  reg [15:0] REVISION = 'h7a09; // z9

  // main FSM
  localparam RESET = 0;
  localparam Z2_CONFIGURING = 1;
  localparam Z2_IDLE = 2;
  localparam WAIT_WRITE = 3;
  localparam WAIT_WRITE2 = 4;
  localparam Z2_WRITE_FINALIZE = 5;
  localparam WAIT_READ = 6;
  localparam WAIT_READ2 = 7;
  localparam WAIT_READ3 = 8;
  
  localparam CONFIGURED = 9;
  localparam CONFIGURED_CLEAR = 10;
  localparam DECIDE_Z2_Z3 = 11;
  
  localparam Z3_IDLE = 12;
  localparam Z3_WRITE_UPPER = 13;
  localparam Z3_WRITE_LOWER = 14;
  localparam Z3_READ_UPPER = 15;
  localparam Z3_READ_LOWER = 16;
  localparam Z3_READ_DELAY = 17;
  localparam Z3_READ_DELAY1 = 18;
  localparam Z3_READ_DELAY2 = 19;
  localparam Z3_WRITE_PRE = 20;
  localparam Z3_WRITE_FINALIZE = 21;
  localparam Z3_ENDCYCLE = 22;
  localparam Z3_DTACK = 23;
  localparam Z3_CONFIGURING = 24;
  
  localparam Z2_REGWRITE = 25;
  localparam REGWRITE = 26;
  localparam REGREAD = 27;
  localparam Z2_REGREAD_POST = 28;
  localparam Z3_REGREAD_POST = 29;
  localparam Z3_REGWRITE = 30;
  localparam Z2_REGREAD = 31;
  localparam Z3_REGREAD = 32;
  
  localparam Z2_PRE_CONFIGURED = 34;
  localparam Z2_ENDCYCLE = 35;
  
  localparam WAIT_WRITE_DMA_Z2 = 36;
  localparam WAIT_WRITE_DMA_Z2_FINALIZE = 37;
  
  localparam RESET_DVID = 39;
  localparam COLD = 40;
  
  localparam WAIT_READ2B = 41; // delay states
  localparam WAIT_READ2C = 42;
  
  localparam WAIT_WRITE_DMA_Z3 = 43;
  localparam WAIT_WRITE_DMA_Z3_FINALIZE = 44;
  
  localparam Z3_AUTOCONF_READ = 45;
  localparam Z3_AUTOCONF_WRITE = 46;
  localparam Z3_AUTOCONF_READ_DLY = 47;
  localparam Z3_AUTOCONF_READ_DLY2 = 48;
  
  localparam Z3_REGWRITE_PRE = 49;
  localparam Z3_REGREAD_PRE = 50;
  localparam Z3_WRITE_PRE2 = 51;
  
  reg [7:0] zorro_state = COLD;
  reg zorro_idle = 0;
  reg [7:0] read_counter = 0; // used by Z3
  reg [7:0] dataout_time = 'h02;
  reg [7:0] datain_time = 'h10;
  reg [7:0] datain_counter = 0;
  reg [23:0] last_addr = 0;
  reg [23:0] last_read_addr = 0;
  reg [15:0] last_data = 0;
  reg [15:0] last_read_data = 0;
 
  reg [15:0] zaddr_regpart = 0;
  reg [15:0] z3addr_regpart = 0;
  reg [15:0] regread_addr = 0;
  reg [15:0] regwrite_addr = 0;
  
  reg [15:0] count_writes = 0;
  reg [15:0] testreg_1 = 0;
  
  reg zorro_ram_read_flag = 0;
  reg zorro_ram_write_flag = 0;
  
  reg videocap_mode = 0;
  reg [9:0] videocap_hs = 0;
  reg [9:0] videocap_vs = 0;
  reg [2:0] videocap_state = 0;
  reg [23:0] videocap_rgbin = 0;
  reg [23:0] videocap_rgbin2 = 0;
  
  reg [9:0] videocap_x = 0;
  reg [9:0] videocap_x2 = 0;
  reg [9:0] videocap_y = 0;
  reg [9:0] videocap_y2 = 0;
  reg [9:0] videocap_ymax = 0;
  reg [9:0] videocap_y3 = 0;
  reg [9:0] videocap_voffset = 'h1a; //'h2a;
  reg [9:0] videocap_prex_in = 'h2d; //'h42;
  reg [9:0] videocap_prex = 'h2d; //'h42;
  reg [9:0] videocap_height = 'h200; //'h117; // 'h127;
  reg [8:0] videocap_width = 360; // 320
  
  parameter VCAPW = 399;
  reg [31:0] videocap_buf [0:VCAPW];
  reg [31:0] videocap_buf2 [0:VCAPW];
  reg videocap_lace_field=0;
  reg videocap_interlace=0;
  reg videocap_ntsc=0;
  reg [9:0] videocap_voffset2=0;
  
  always @(posedge ZORRO_E7M) begin
    videocap_hs <= {videocap_hs[8:0], VCAP_HSYNC};
    videocap_vs <= {videocap_vs[8:0], VCAP_VSYNC};
    
    /*videocap_rgbin <=  {VCAP_R5,VCAP_R4,VCAP_R3,VCAP_R2,4'b0000,
                        VCAP_G5,VCAP_G4,VCAP_G3,VCAP_G2,4'b0000,
                        VCAP_B5,VCAP_B4,VCAP_B3,VCAP_B2,4'b0000};*/
                        
    videocap_rgbin <=  {VCAP_R7,VCAP_R6,VCAP_R5,VCAP_R4,VCAP_R3,VCAP_R2,VCAP_R1,VCAP_R0,
                        VCAP_G7,VCAP_G6,VCAP_G5,VCAP_G4,VCAP_G3,VCAP_G2,VCAP_G1,VCAP_G0,
                        VCAP_B7,VCAP_B6,VCAP_B5,VCAP_B4,VCAP_B3,VCAP_B2,VCAP_B1,VCAP_B0};
                        
    videocap_prex <= videocap_prex_in;
                       
    //if (!videocap_mode) begin
      // do nothing
    //end else 
    if (videocap_vs[6:1]=='b111000) begin
      if (videocap_y2>1) begin
        if (videocap_ymax=='h270 || videocap_ymax=='h20c)
          videocap_lace_field <= 0;
        else
          videocap_lace_field <= ~videocap_lace_field;
          
        if (videocap_ymax=='h138 || videocap_ymax=='h271) begin
          videocap_interlace <= 1;
          videocap_ntsc <= 0;
        end else if (videocap_ymax=='h106 || videocap_ymax=='h20d) begin
          videocap_interlace <= 1;
          videocap_ntsc <= 1;
        end else if (videocap_ymax=='h273) begin
          videocap_interlace <= 0;
          videocap_ntsc <= 0;
        end else  if (videocap_ymax=='h20f) begin
          videocap_interlace <= 0;
          videocap_ntsc <= 1;
        end
        
        if (videocap_interlace) begin
          videocap_y2 <= videocap_lace_field;
          videocap_voffset2 <= videocap_voffset<<1;
        end else begin
          videocap_y2 <= 0;
          videocap_voffset2 <= videocap_voffset;
        end
        
        videocap_ymax <= videocap_y2;
      end
    end else if (videocap_hs[6:1]=='b000111) begin
      videocap_x <= 0;
      if (videocap_interlace)
        videocap_y2 <= videocap_y2 + 2'b10;
      else
        videocap_y2 <= videocap_y2 + 1'b1;
    end else if (videocap_x<VCAPW) begin
      videocap_x <= videocap_x + 1'b1;
      videocap_buf2[videocap_x-videocap_prex] <= {8'b0,videocap_rgbin};
    end
  end
    
  always @(negedge ZORRO_E7M) begin
    /*videocap_rgbin2 <= {VCAP_R5,VCAP_R4,VCAP_R3,VCAP_R2,4'b0000,
                        VCAP_G5,VCAP_G4,VCAP_G3,VCAP_G2,4'b0000,
                        VCAP_B5,VCAP_B4,VCAP_B3,VCAP_B2,4'b0000};*/
                        
    videocap_rgbin2 <= {VCAP_R7,VCAP_R6,VCAP_R5,VCAP_R4,VCAP_R3,VCAP_R2,VCAP_R1,VCAP_R0,
                        VCAP_G7,VCAP_G6,VCAP_G5,VCAP_G4,VCAP_G3,VCAP_G2,VCAP_G1,VCAP_G0,
                        VCAP_B7,VCAP_B6,VCAP_B5,VCAP_B4,VCAP_B3,VCAP_B2,VCAP_B1,VCAP_B0};
                         
    /*videocap_rgbin2 <=  {VCAP_B7,VCAP_B6,VCAP_B5,VCAP_B4,VCAP_B3,VCAP_B2,VCAP_B1,VCAP_B0,
                        VCAP_G7,VCAP_G6,VCAP_G5,VCAP_G4,VCAP_G3,VCAP_G2,VCAP_G1,VCAP_G0,
                        VCAP_R7,VCAP_R6,VCAP_R5,VCAP_R4,VCAP_R3,VCAP_R2,VCAP_R1,VCAP_R0};*/
                        
    //if (!videocap_mode) begin
      // do nothing
    //end else 
    
    if (videocap_hs[6:1]=='b000111) begin
      videocap_x2 <= 0;
    end else if (videocap_x2<VCAPW) begin
      videocap_x2 <= videocap_x2 + 1'b1;
      videocap_buf[videocap_x2-videocap_prex] <= videocap_rgbin2;
    end
  end
  
  reg [11:0] videocap_save_x=0;
  reg [11:0] videocap_save_x2=0;
  reg [11:0] videocap_yoffset=0;
  reg [11:0] videocap_xoffset=0;
  reg [11:0] videocap_pitch=720;
  reg [9:0] videocap_save_line_done=1;
  reg [11:0] videocap_save_y=0;
  reg [31:0] videocap_save_y2=0;
  reg [31:0] videocap_save_addr=0;
  reg [3:0] videocap_save_state=0; // FIXME
  
  reg m00_axi_awready_reg;
  reg m00_axi_wready_reg;
  
  reg videocap_mode_sync;
  
  always @(posedge S_AXI_ACLK) begin
    // VIDEOCAP
    videocap_mode_sync <= videocap_mode;
    
    m00_axi_awlen <= 'h0; // 1 burst (1 write)
    m00_axi_awsize <= 'h2; // 2^2 == 4 bytes
    m00_axi_awburst <= 'h0; // FIXED (non incrementing)
    m00_axi_awcache <= 'h1;
    m00_axi_awlock <= 'h0;
    m00_axi_awprot <= 'h0;
    m00_axi_awqos <= 'h0;
    m00_axi_wlast <= 'h1;
    m00_axi_bready <= 'h1;
    
    m00_axi_awready_reg <= m00_axi_awready;
    m00_axi_wready_reg <= m00_axi_wready;
    
    videocap_save_x2 <= (videocap_save_x);
    videocap_save_y2 <= (videocap_y2-videocap_voffset2);
    videocap_save_addr <= (videocap_save_y2)*videocap_pitch+videocap_save_x2;
    
    video_control_interlace <= videocap_interlace;
    
    if (videocap_mode_sync) begin
      m00_axi_wstrb <= 4'b1111;
      m00_axi_awaddr <= 'hf10000+(videocap_save_addr<<2); // <<2 = *4 FIXME select sane area and protect it
      
      // FIXME for some computers (?) buf and buf2 are swapped... some timing race condition
      if (videocap_save_x[0])
        m00_axi_wdata  <= videocap_buf2[videocap_save_x>>1];
      else
        m00_axi_wdata  <= videocap_buf[videocap_save_x>>1];
      
      // save newly captured line
      case (videocap_save_state)
        0:
          if (videocap_save_line_done!=videocap_y2 && videocap_x>8) begin
            m00_axi_awvalid <= 1;
            m00_axi_wvalid <= 1;
            if (m00_axi_awready) begin
              videocap_save_state <= 1;
            end
          end
        1: begin
          m00_axi_awvalid <= 0;
          m00_axi_wvalid <= 0;
          videocap_save_x <= videocap_save_x + 1'b1;
          if (videocap_save_x > 722) begin // FIXME was 720
            videocap_save_line_done <= videocap_y2;
            videocap_save_x <= 0;
          end
          videocap_save_state <= 2;
        end
        2: begin
          //if (m00_axi_bvalid) begin
            videocap_save_state <= 0;
          //end
        end
      endcase
    end
  end
  
  // -- main zorro fsm ---------------------------------------------
  always @(posedge S_AXI_ACLK) begin
    zorro_idle <= ((zorro_state==Z2_IDLE)||(zorro_state==Z3_IDLE));
    
    if (/*z_cfgin_lo ||*/ z_reset) begin
      zorro_state <= RESET;
    end else case (zorro_state)
      COLD: begin
        zorro_state <= RESET;
      end
      
      RESET: begin
        dataout_enable <= 0;
        dataout <= 0;
        dataout_z3 <= 0;
        slaven <= 0;
        z_ovr <= 0;
        z_confout <= 0;
        z3_confdone <= 0;
        
        if (!z_reset)
          zorro_state <= DECIDE_Z2_Z3;
        
        count_writes <= 0;
        videocap_mode <= 1;
        last_z3addr <= 0;
        
        // RESET video controller
        //video_control_op <= 11;
      end
      
      DECIDE_Z2_Z3: begin
        //video_control_op <= 0;
        
        /*if (zaddr_autoconfig) begin
          ZORRO3 <= 0;
          zorro_state <= Z2_CONFIGURING;
        end*/
        if (z3addr_autoconfig) begin
          //ZORRO3 <= 1;
          zorro_state <= Z3_CONFIGURING;
          //z_confout <= 1;
          //zorro_state <= CONFIGURED;
        end
      end
      
      Z3_AUTOCONF_READ_DLY: begin
        // wait for data to be latched out
        zorro_state <= Z3_AUTOCONF_READ_DLY2;
      end
      
      Z3_AUTOCONF_READ_DLY2: begin
        // wait for data to be latched out
        zorro_state <= Z3_DTACK;
      end
      
      Z3_AUTOCONF_READ: begin
        dataout_z3 <= 1;
        slaven <= 1;
        zorro_state <= Z3_AUTOCONF_READ_DLY;
        
        last_z3addr <= z3addr;
        
        case (z3addr[15:0])
          'h0000: data_z3_hi16 <= 'b1000_1111_1111_1111; // zorro 3 (10), no pool link (0), autoboot ROM (1)
          'h0100: data_z3_hi16 <= 'b0100_1111_1111_1111; // next board unrelated (0), 256MB
          
          'h0004: data_z3_hi16 <= 'b1111_1111_1111_1111; // product number
          'h0104: data_z3_hi16 <= 'b1011_1111_1111_1111; // (4)
          
          'h0008: data_z3_hi16 <= 'b0000_1111_1111_1111; // flags inverted 0111 io,shutup,extension,reserved(1)
          'h0108: data_z3_hi16 <= 'b1111_1111_1111_1111; // inverted zero
          
          'h000c: data_z3_hi16 <= 'b1111_1111_1111_1111; // reserved?
          'h010c: data_z3_hi16 <= 'b1111_1111_1111_1111; // 
          
          'h0010: data_z3_hi16 <= 'b1001_1111_1111_1111; // manufacturer high byte inverted
          'h0110: data_z3_hi16 <= 'b0010_1111_1111_1111; // 
          'h0014: data_z3_hi16 <= 'b1001_1111_1111_1111; // manufacturer low byte
          'h0114: data_z3_hi16 <= 'b0001_1111_1111_1111;
          
          'h0018: data_z3_hi16 <= 'b1111_1111_1111_1111; // serial 01 01 01 01
          'h0118: data_z3_hi16 <= 'b1110_1111_1111_1111; //
          'h001c: data_z3_hi16 <= 'b1111_1111_1111_1111; //
          'h011c: data_z3_hi16 <= 'b1110_1111_1111_1111; //
          'h0020: data_z3_hi16 <= 'b1111_1111_1111_1111; //
          'h0120: data_z3_hi16 <= 'b1110_1111_1111_1111; //
          'h0024: data_z3_hi16 <= 'b1111_1111_1111_1111; //
          'h0124: data_z3_hi16 <= 'b1110_1111_1111_1111; //
          
          /*'h0028: data_z3_hi16 <= 'b1111_1111_1111_1111; // autoboot rom vector (er_InitDiagVec)
          'h0128: data_z3_hi16 <= 'b1111_1111_1111_1111; // ff7f = ~0080
          'h002c: data_z3_hi16 <= 'b0111_1111_1111_1111;
          'h012c: data_z3_hi16 <= 'b1111_1111_1111_1111;*/
         
          default: data_z3_hi16 <= 'b1111_1111_1111_1110; // FIXME
        endcase
      end
      
      Z3_AUTOCONF_WRITE: begin
        slaven <= 1;
        // was [2] for A3000
        if (z3_din_latch) begin
          zorro_state <= Z3_DTACK;
          casex (z3addr[15:0])
            'hXX44: begin
              z3_ram_low[31:16] <= zdata_in_sync;
              z_confout <= 1;
              z3_confdone <= 1;
            end
            'hXX48: begin
            end
            'hXX4c: begin
              // shutup
              z_confout <= 1;
              z3_confdone <= 1;
            end
          endcase
        end
      end
      
      Z3_CONFIGURING: begin
        data_z3_low16 <= 'hffff;
        
        // was [2] for A3000
        if (z_cfgin && z3addr_autoconfig) begin
          if (zorro_read) begin
            // autoconfig ROM
            zorro_state <= Z3_AUTOCONF_READ;
            
          end else begin
            // write to autoconfig register
            zorro_state <= Z3_AUTOCONF_WRITE;
          end
        end
        
        dataout_z3 <= 0;
        slaven <= 0;
        dtack <= 0;
      end
      
      Z3_DTACK: begin
        if (z3_end_cycle) begin
          dtack <= 0;
          dataout_z3 <= 0;
          slaven <= 0;
          if (z3_confdone) begin
            zorro_state <= CONFIGURED;
          end else
            zorro_state <= Z3_CONFIGURING;
        end else
          dtack <= 1;
      end
      
      CONFIGURED: begin
        ram_high <= ram_low + `RAM_SIZE;
        reg_low <= ram_low + 'h1000;
        reg_high <= ram_low + 'h2000;
        
        z3_ram_high  <= z3_ram_low + `Z3_RAM_SIZE;
        z3_reg_low   <= z3_ram_low + 'h1000;
        z3_reg_high  <= z3_ram_low + 'h2000;
        
        zorro_state <= CONFIGURED_CLEAR;
      end
      
      CONFIGURED_CLEAR: begin
        if (ZORRO3)
          zorro_state <= Z3_IDLE;
        //else
        //  zorro_state <= Z2_IDLE;
      end
      
      // ---------------------------------------------------------------------------------
      `ifdef ZORRO2_SUPPORT
      Z2_CONFIGURING: begin
        z_ovr <= 0;
        if (zaddr_autoconfig && z_cfgin) begin
          if (z2_read) begin
            // read iospace 'he80000 (Autoconfig ROM)
            dataout_enable <= 1;
            dataout <= 1;
            slaven <= 1;
            
            case (z2_mapped_addr[7:0])
              8'h00: data_out <= 'b1101_1111_1111_1111; // zorro 2 (11), no pool (0) rom (1)
              8'h02: data_out <= 'b0111_1111_1111_1111; // next board unrelated (0), 4mb (110 for 2mb)
              
              8'h04: data_out <= 'b1111_1111_1111_1111; // product number
              8'h06: data_out <= 'b1100_1111_1111_1111; // (3)
              
              8'h08: data_out <= 'b0011_1111_1111_1111; // flags inverted 0011
              8'h0a: data_out <= 'b1110_1111_1111_1111; // inverted 0001 = OS sized
              
              8'h10: data_out <= 'b1001_1111_1111_1111; // manufacturer high byte inverted (02)
              8'h12: data_out <= 'b0010_1111_1111_1111; // 
              8'h14: data_out <= 'b1001_1111_1111_1111; // manufacturer low byte (9a)
              8'h16: data_out <= 'b0001_1111_1111_1111;
              
              8'h18: data_out <= 'b1111_1111_1111_1111; // serial 01 01 01 01
              8'h1a: data_out <= 'b1110_1111_1111_1111; //
              8'h1c: data_out <= 'b1111_1111_1111_1111; //
              8'h1e: data_out <= 'b1110_1111_1111_1111; //
              8'h20: data_out <= 'b1111_1111_1111_1111; //
              8'h22: data_out <= 'b1110_1111_1111_1111; //
              8'h24: data_out <= 'b1111_1111_1111_1111; //
              8'h26: data_out <= 'b1110_1111_1111_1111; //
              
              8'h28: data_out <= 'b1111_1111_1111_1111; // autoboot rom vector (er_InitDiagVec)
              8'h2a: data_out <= 'b1111_1111_1111_1111; // ff7f = ~0080
              8'h2c: data_out <= 'b0111_1111_1111_1111;
              8'h2e: data_out <= 'b1111_1111_1111_1111;
              
              //'h000040: data <= 'b0000_0000_0000_0000; // interrupts (not inverted)
              //'h000042: data <= 'b0000_0000_0000_0000; //
              
              default: data_out <= 'b1111_1111_1111_1111;
            endcase
          end else begin
            // write to autoconfig register
            if (datastrobe_synced) begin
              case (z2_mapped_addr[7:0])
                8'h48: begin
                  ram_low[31:24] <= 8'h0;
                  ram_low[23:20] <= zdata_in_sync[15:12];
                  ram_low[15:0] <= 16'h0;
                  zorro_state <= Z2_PRE_CONFIGURED; // configured
                end
                8'h4a: begin
                  ram_low[31:24] <= 8'h0;
                  ram_low[19:16] <= zdata_in_sync[15:12];
                  ram_low[15:0] <= 16'h0;
                end
                
                8'h4c: begin 
                  zorro_state <= Z2_PRE_CONFIGURED; // configured, shut up
                end
              endcase
            end
          end
        end else begin
          // no address match
          dataout <= 0;
          dataout_enable <= 0;
          slaven <= 0;
        end
      end
      Z2_PRE_CONFIGURED: begin
        if (znAS_sync[2]==1) begin
          z_confout<=1;
          zorro_state <= CONFIGURED;
        end
      end
      Z2_IDLE: begin
        if (z2_addr_valid) begin
            
          if (z2_write && zaddr_in_reg) begin
            // write to register
            dataout_enable <= 0;
            dataout <= 0;
            z_ovr <= 1;
            zaddr_regpart <= z2_mapped_addr;
            zorro_state <= Z2_REGWRITE;
            
          end else if (z2_read && zaddr_in_reg) begin
            // read from registers
            dataout_enable <= 1;
            dataout <= 1;
            data_out <= default_data; //'hffff;
            slaven <= 1;
            z_ovr <= 1;
            zaddr_regpart <= z2_mapped_addr;
            zorro_state <= Z2_REGREAD;
            
          end else if (z2_read && zaddr_in_ram) begin
            // read RAM
            // request ram access from arbiter
            last_addr <= z2_mapped_addr-ram_low;
            data_out <= default_data; //'hffff;
            slaven <= 1;
            dataout_enable <= 1;
            dataout <= 1;
            z_ovr <= 1;
            zorro_state <= WAIT_READ3;
            
          end else if (z2_write && zaddr_in_ram) begin
            // write RAM
            last_addr <= z2_mapped_addr-ram_low;
            dataout_enable <= 0;
            dataout <= 0;
            datain_counter <= 0;
            z_ovr <= 1;
            count_writes <= count_writes + 1;
            zorro_state <= WAIT_WRITE;
            
          end else begin
            dataout <= 0;
            dataout_enable <= 0;
            slaven <= 0;
          end
          
        end else begin
          dataout <= 0;
          dataout_enable <= 0;
          slaven <= 0;
        end
      end
      Z2_REGWRITE: begin
        if (datastrobe_synced) begin
          regdata_in <= zdata_in_sync;
          regwrite_addr <= zaddr_regpart;
          zorro_state <= REGWRITE;
        end
      end
      WAIT_READ3: begin
        zorro_ram_read_addr <= last_addr;
        zorro_ram_read_request <= 1;
        zorro_state <= WAIT_READ2;
      end
      WAIT_READ2: begin
        if (!zorro_ram_read_request && !zorro_ram_read_flag) begin
          data_out <= slv_reg1[15:0];
        
          zorro_state <= WAIT_READ2B;
        end
      end
      WAIT_READ2B: begin
        zorro_state <= WAIT_READ2C;
      end
      WAIT_READ2C: begin
        zorro_state <= Z2_ENDCYCLE;
      end
      WAIT_WRITE: begin
        if (!zorro_ram_write_request && !zorro_ram_write_flag) begin
          if (datastrobe_synced) begin
            zorro_write_capture_bytes <= {~znUDS_sync[1],~znLDS_sync[1]};
            zorro_write_capture_data <= zdata_in_sync; //_sync;
            
            if (last_addr<'h10000 || videocap_mode)
              zorro_state <= WAIT_WRITE2;
            else
              zorro_state <= WAIT_WRITE_DMA_Z2;
          end
        end
      end
      WAIT_WRITE2: begin
        zorro_ram_write_addr <= last_addr;
        zorro_ram_write_bytes <= {2'b0,zorro_write_capture_bytes};
        zorro_ram_write_data <= {16'b0,zorro_write_capture_data};
        zorro_ram_write_request <= 1;
        zorro_state <= Z2_WRITE_FINALIZE;
      end
      /*WAIT_WRITE_DMA_Z2: begin
        if (last_addr[1])
          m00_axi_wstrb <= {zorro_write_capture_bytes[0],zorro_write_capture_bytes[1],2'b0};
        else
          m00_axi_wstrb <= {2'b0,zorro_write_capture_bytes[0],zorro_write_capture_bytes[1]};
        
        // FIXME
        m00_axi_awaddr <= (last_addr+`ARM_MEMORY_START)&'hfffffc;
        m00_axi_wdata  <= {zorro_write_capture_data[7:0],zorro_write_capture_data[15:8],zorro_write_capture_data[7:0],zorro_write_capture_data[15:8]};
        m00_axi_awvalid <= 1;
        m00_axi_wvalid  <= 1;
        if (m00_axi_awready) begin  // TODO wready?
          zorro_state <= WAIT_WRITE_DMA_Z2_FINALIZE;
        end
      end*/
      /*WAIT_WRITE_DMA_Z2_FINALIZE: begin
        if (m00_axi_wready) begin
          m00_axi_awvalid <= 0;
          m00_axi_wvalid <= 0;
          zorro_state <= Z2_ENDCYCLE;
        end
      end*/
      Z2_WRITE_FINALIZE: begin
        // FIXME CHECK
        //if (!zorro_ram_write_request) begin
          zorro_state <= Z2_ENDCYCLE;
        //end
      end
      Z2_ENDCYCLE: begin
        if (!z2_addr_valid) begin
          dtack <= 0;
          slaven <= 0;
          dataout_enable <= 0;
          dataout <= 0;
          z_ovr <= 0;
          zorro_state <= Z2_IDLE;
        end else
          dtack <= 1;
      end
      // 16bit reg read
      Z2_REGREAD_POST: begin
        if (zaddr_regpart[1]==1'b1)
          data_out <= rr_data[15:0];
        else
          data_out <= rr_data[31:16];
        zorro_state <= Z2_ENDCYCLE;
      end
      // relaxing the data pipeline a bit
      Z2_REGREAD: begin
        regread_addr <= zaddr_regpart;
        zorro_state <= REGREAD;
      end
      `endif
      
      // =========================================================================
      // ZORRO 3
      // =========================================================================
      
      Z3_REGWRITE_PRE: begin
        if (znDS1_sync[1]==0) begin
          regdata_in <= data_in_z3_low16;
          z3addr_regpart <= (z3addr[15:0])|16'h2;
          zorro_state <= Z3_REGWRITE;
        end else if (znUDS_sync[1]==0) begin
          regdata_in <= zdata_in_sync;
          z3addr_regpart <= z3addr[15:0];
          zorro_state <= Z3_REGWRITE;
        end
      end
      
      Z3_REGREAD_PRE: begin
        z3addr_regpart <= z3addr[15:0]; //|16'h2;
        if (z3_din_latch) begin
          zorro_state <= Z3_REGREAD;
        end
      end
      
      Z3_IDLE: begin
        if (z3_fcs_state==0) begin
          // falling edge of /FCS
          
          if (zorro_write && z3addr_in_reg) begin
            // FIXME doesn't support 32 bit access
            // write to register
            slaven <= 1;
            dataout_z3 <= 0;
            zorro_state <= Z3_REGWRITE_PRE;
          end else if (zorro_read && z3addr_in_reg) begin
            // read registers
            slaven <= 1;
            dataout_z3 <= 1;
            data_z3_hi16 <= default_data;
            data_z3_low16 <= default_data;
            zorro_state <= Z3_REGREAD_PRE;
          end else if (z3addr_in_ram && zorro_write) begin
            // write to memory
            slaven <= 1;
            dataout_z3 <= 0;
            read_counter <= 0;
            
            //if (z3_din_latch) begin
              zorro_state <= Z3_WRITE_PRE;
            //end
          end else if (z3addr_in_ram && zorro_read) begin
            // read from memory
            slaven <= 1;
            data_z3_hi16  <= default_data;
            data_z3_low16 <= default_data;
            dataout_z3 <= 1;
            dtack <= 0;
        
            zorro_state <= Z3_READ_UPPER;
          
          end else begin
            // address not recognized
            slaven <= 0;
            dtack <= 0;
            dataout_z3 <= 0;
          end
          
        end else begin
          // not in a cycle
          slaven <= 0;
          dtack <= 0;
          dataout_z3 <= 0;
        end
      end
      
      Z3_REGWRITE: begin
        regwrite_addr <= z3addr_regpart;
        zorro_state <= REGWRITE;
      end
      
      Z3_REGREAD: begin
        regread_addr <= z3addr_regpart;
        zorro_state <= REGREAD;
      end
      
      // 32bit reg read
      Z3_REGREAD_POST: begin
        data_z3_hi16  <= rr_data[31:16];
        data_z3_low16 <= rr_data[15:0];
        zorro_state <= Z3_READ_DELAY2;
        read_counter <= 0;
      end
        
      Z3_READ_UPPER: begin
        last_z3addr <= z3_mapped_addr;
      
        zorro_state <= Z3_READ_DELAY1;
        zorro_ram_read_addr <= z3_mapped_addr;
        zorro_ram_read_bytes <= 4'b1111;
        zorro_ram_read_request <= 1;
      end
      
      Z3_READ_DELAY1: begin
        data_z3_hi16 <= slv_reg1[31:16];
        data_z3_low16 <= slv_reg1[15:0];
        read_counter <= 0;
          
        if (!zorro_ram_read_request && !zorro_ram_read_flag) begin
          zorro_state <= Z3_ENDCYCLE; // CHECK DELAY
          //zorro_state <= Z3_READ_DELAY2; // CHECK DELAY
        end
      end
      
      Z3_READ_DELAY2: begin
        if (read_counter >= dataout_time)
          zorro_state <= Z3_ENDCYCLE;
        else
          read_counter <= read_counter+1'b1;
      end
      
      Z3_WRITE_PRE2: begin
        // 1 more time for good measure
        z3_ds0 <= ~znDS0_sync[1];
        z3_ds1 <= ~znDS1_sync[1];
        z3_ds2 <= ~znLDS_sync[1];
        z3_ds3 <= ~znUDS_sync[1];
        
        zorro_state <= Z3_WRITE_UPPER;
      end
      
      Z3_WRITE_PRE: begin
        // FIXME was [2] for A3000
        z3_ds0 <= ~znDS0_sync[1];
        z3_ds1 <= ~znDS1_sync[1];
        z3_ds2 <= ~znLDS_sync[1];
        z3_ds3 <= ~znUDS_sync[1];
        
        if (~znDS0_sync[1]||~znDS1_sync[1]||~znLDS_sync[1]||~znUDS_sync[1]) begin
          //if (z3_mapped_addr<'h10000 || videocap_mode)
            zorro_state <= Z3_WRITE_PRE2;
          //else
          //  zorro_state <= WAIT_WRITE_DMA_Z3;
        end
      end
      
      Z3_WRITE_UPPER: begin
        if (!zorro_ram_write_request && !zorro_ram_write_flag) begin
          zorro_ram_write_addr <= z3_mapped_addr;
          last_z3addr <= z3_mapped_addr;
          zorro_ram_write_bytes <= {z3_ds3,z3_ds2,z3_ds1,z3_ds0};
          zorro_ram_write_data  <= {z3_din_high_s2,z3_din_low_s2};
          zorro_ram_write_request <= 1;
          
          zorro_state <= Z3_WRITE_FINALIZE;
        end
      end
      
      /*WAIT_WRITE_DMA_Z3: begin
        m00_axi_wstrb <= {z3_ds0,z3_ds1,z3_ds2,z3_ds3};
        // FIXME
        m00_axi_awaddr <= (z3_mapped_addr&'h1ffffffc)+`ARM_MEMORY_START; // max 256MB
        m00_axi_wdata  <= {z3_din_low_s2[7:0],z3_din_low_s2[15:8],z3_din_high_s2[7:0],z3_din_high_s2[15:8]};
        m00_axi_awvalid <= 1;
        m00_axi_wvalid  <= 1;
        if (m00_axi_awready) begin  // TODO wready?
          zorro_state <= WAIT_WRITE_DMA_Z3_FINALIZE;
        end
      end*/
      
      /*WAIT_WRITE_DMA_Z3_FINALIZE: begin
        if (m00_axi_wready) begin
          m00_axi_awvalid <= 0;
          m00_axi_wvalid <= 0;
          zorro_state <= Z3_ENDCYCLE;
          //dtack <= 1; // CHECKME
        end
      end*/
      
      Z3_WRITE_FINALIZE: begin
        dtack <= 1;
        zorro_state <= Z3_ENDCYCLE;
      end
        
      Z3_ENDCYCLE: begin
        if (z3_end_cycle) begin
          dtack <= 0;
          slaven <= 0;
          dataout_z3 <= 0;
          zorro_state <= Z3_IDLE;
        end else
          dtack <= 1;
      end
      
      // FIXME why is there no dataout time on REGREAD? (see memory reads)
      // now fixed for Z3, still pending for Z2
      REGREAD: begin
        // TODO split up into z3/z2
        //if (ZORRO3) begin
          zorro_state <= Z3_REGREAD_POST;
        //end else begin
        //  zorro_state <= Z2_REGREAD_POST;
        //end
        case (regread_addr&'hff)
          /*'h00: begin
            rr_data <= video_control_data; 
            end
          'h04: begin
            rr_data <= video_control_op; 
            end*/
          'h00: begin
            // this flag is read by Amiga software to check if all writes are done
            rr_data <= zorro_ram_write_request;
          end
          default: begin
            rr_data[31:16] <= REVISION;
            rr_data[15:0]  <= REVISION;
          end
        endcase
      end
      
      REGWRITE: begin
        // TODO split up into z3/z2
        //if (ZORRO3) begin
          zorro_state <= Z3_ENDCYCLE;
        //end else
        //  zorro_state <= Z2_ENDCYCLE;
        
        case (regwrite_addr&'hff)
          'h00: video_control_data[31:16] <= regdata_in[15:0];
          'h02: video_control_data[15:0]  <= regdata_in[15:0];
          'h04: video_control_op[7:0]     <= regdata_in[7:0]; // FIXME
          'h06: videocap_mode <= regdata_in[0];
          //'h08: videocap_prex_in <= regdata_in;
        endcase
      end
    endcase
    
    if (slv_reg0[31]==1'b1) begin
      zorro_ram_write_request <= 0;
    end
    if (slv_reg0[30]==1'b1) begin
      zorro_ram_read_request <= 0;
    end
    // ARM video control
    if (slv_reg2[31]==1'b1) begin
      video_control_data <= slv_reg3[31:0];
      video_control_op <= slv_reg2[7:0];
    end
    
    zorro_ram_read_flag <= slv_reg0[30];
    zorro_ram_write_flag <= slv_reg0[31];
    
    out_reg0 <= ZORRO3 ? last_z3addr : last_addr;
    out_reg1 <= zorro_ram_write_data;
    out_reg2 <= last_z3addr;
    out_reg3 <= {zorro_ram_write_request, zorro_ram_read_request, zorro_ram_write_bytes, ZORRO3, video_control_interlace, 16'b0, zorro_state};
  end


	// Implement memory mapped register select and read logic generation
	// Slave register read enable is asserted when valid address is available
	// and the slave is ready to accept the read address.
	assign slv_reg_rden = axi_arready & S_AXI_ARVALID & ~axi_rvalid;
	always @(*)
	  begin
	    // Address decoding for reading registers
	    case ( axi_araddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] )
	      2'h0   : reg_data_out <= out_reg0;
	      2'h1   : reg_data_out <= out_reg1;
	      2'h2   : reg_data_out <= out_reg2;
        2'h3   : reg_data_out <= out_reg3;
	      default : reg_data_out <= 'h0;
	    endcase
	  end

endmodule
