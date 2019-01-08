
`timescale 1 ns / 1 ps

`define C_S_AXI_DATA_WIDTH 32
`define C_S_AXI_ADDR_WIDTH 4
`define RAM_SIZE 32'h400000
`define REG_SIZE 32'h01000
`define AUTOCONF_LOW  24'he80000
`define AUTOCONF_HIGH 24'he80080
`define Z3_RAM_SIZE 32'h02000000

module MNTZorro_v0_1_S00_AXI
	(
	 // Users to add ports here
   inout wire [22:0] ZORRO_ADDR,
   inout wire [15:0] ZORRO_DATA,
   
   output wire ZORRO_DATADIR,
   output wire ZORRO_ADDRDIR,

   input wire ZORRO_READ,        
   input wire ZORRO_NMTCR,
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
  
   output wire ZORRO_NCFGOUT,
   output wire ZORRO_NSLAVE,
   output wire ZORRO_NCINH,
   output wire ZORRO_NDTACK,
  
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
  
  reg [2:0] znFCS_sync = 3'b111;
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
  reg [31:0] ram_low = 32'h600000;
  reg [31:0] ram_high = 32'ha00000;
  reg [31:0] reg_low  = 32'h600000;
  reg [31:0] reg_high = 32'h601000;
  reg z2_uds = 0;
  reg z2_lds = 0;

  reg [31:0] z3_ram_low = 32'h48000000;
  reg [31:0] z3_ram_high = 32'h48000000 + `Z3_RAM_SIZE - 'h10000-4;
  reg [31:0] z3_reg_low  = 32'h48000000 + `Z3_RAM_SIZE - 'h10000;
  reg [31:0] z3_reg_high = 32'h48000000 + `Z3_RAM_SIZE - 'h10000 + `REG_SIZE;
  reg [15:0] data_z3_hi16;
  reg [15:0] data_z3_low16;
  
  reg [15:0] data_z3_hi16_latched;
  reg [15:0] data_z3_low16_latched;
  
  reg [15:0] data_in_z3_low16;
  reg [15:0] z3_din_high_s2;
  reg [15:0] z3_din_low_s2;
  reg [31:0] z3addr;
  reg [31:0] z3addr2;
  reg [31:0] z3addr3;
  reg [31:0] z3_mapped_addr;
  reg [31:0] z3_read_addr;
  reg [15:0] z3_read_data;
  reg z3_din_latch = 0;
  reg z3_fcs_state = 0;
  reg z3_end_cycle = 0;
  
  reg z3addr_in_ram = 0;
  reg z3addr_in_reg = 0;
  reg z3addr_autoconfig = 0;

  reg ZORRO3 = 0;
  reg dataout = 0;
  reg dataout_z3 = 0;
  reg dataout_z3_latched = 0;
  reg dataout_enable = 0;
  reg slaven = 0;
  reg dtack = 0;
  reg dtack_latched = 0;

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
  reg [23:0] zorro_ram_read_addr;
  reg [15:0] zorro_ram_read_data;
  reg [1:0] zorro_ram_read_bytes;
  reg [23:0] zorro_ram_write_addr;
  reg [15:0] zorro_ram_write_data;
  reg [1:0] zorro_ram_write_bytes;


  reg [15:0] default_data = 'hffff; // causes read/write glitches on A2000 (data bus interference) when 0
  reg [1:0] zorro_write_capture_bytes = 0;
  reg [15:0] zorro_write_capture_data = 0;
  
  // level shifter direction pins
  assign ZORRO_DATADIR     = ZORRO_DOE & (dataout_enable); // d2-d9  d10-15, d0-d1
  assign ZORRO_ADDRDIR     = ZORRO_DOE & (dataout_z3_latched); // a16-a23 <- input  a8-a15 <- input
  
  wire ZORRO_DATA_T = ~(ZORRO_DOE & dataout_enable);
  wire ZORRO_ADDR_T = ~(ZORRO_DOE & dataout_z3_latched);
  
  reg z_ovr = 0;
  assign ZORRO_NCINH = z_ovr?1'b1:1'b0; // inverse
  
  assign ZORRO_NSLAVE = (slaven)?1'b0:1'b1;
  assign ZORRO_NDTACK  = (ZORRO_DOE & dtack_latched)?1'b1:1'b0; // inverse
  reg [22:0] z3_addr_out; // = {data_z3_low16_latched, 7'bZZZ_ZZZZ};
  
  //assign ZORRO_DATA  = (ZORRO_DOE & dataout_z3_latched) ? data_z3_hi16_latched : ((ZORRO_DOE & dataout) ? data : 16'bzzzz_zzzz_zzzz_zzzz); // data = Z2: full 16 bit or Z3: upper 16 bit
  //assign ZORRO_ADDR  = (ZORRO_DOE & dataout_z3_latched) ? {data_z3_low16_latched, 7'bzzzz_zzz} : 23'bzzz_zzzz_zzzz_zzzz_zzzz_zzzz;
  //assign ZORRO_DATA = (ZORRO_DATA_T ? data : 16'bZZZZ_ZZZZ_ZZZZ_ZZZZ);
  
  wire [15:0] ZORRO_DATA_IN;
  wire [22:0] ZORRO_ADDR_IN;
  
  //assign ZORRO_ADDR_IN = ZORRO_ADDR;
  //assign ZORRO_ADDR = (ZORRO_ADDR_T ? z3_addr_out : 23'bZZZ_ZZZZ_ZZZZ_ZZZZ_ZZZZ_ZZZZ);
  
  genvar i;
  
  generate
      for (i=0; i < 16; i=i+1) begin : ZORRO_DATABUS
          IOBUF u_iobuf_dq 
          (
              .I  (data_out[i]),
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

  assign ZORRO_NCFGOUT = 1;

  always @(posedge S_AXI_ACLK) begin
  
    znUDS_sync  <= {znUDS_sync[1:0],ZORRO_NUDS};
    znLDS_sync  <= {znLDS_sync[1:0],ZORRO_NLDS};
    znAS_sync   <= {znAS_sync[1:0],ZORRO_NCCS};
    zREAD_sync  <= {zREAD_sync[1:0],ZORRO_READ};
    
    znDS1_sync  <= {znDS1_sync[1:0],ZORRO_NDS1};
    znDS0_sync  <= {znDS0_sync[1:0],ZORRO_NDS0};
    zDOE_sync   <= {zDOE_sync[0],ZORRO_DOE};
    znFCS_sync <= {znFCS_sync[1:0],ZORRO_NFCS};
    
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
    zaddr_in_reg <= 0; //(z2_mapped_addr>=reg_low && z2_mapped_addr<reg_high);
    
    if (znAS_sync[1]==0 && zaddr_sync2>=`AUTOCONF_LOW && zaddr_sync2<`AUTOCONF_HIGH)
      zaddr_autoconfig <= 1'b1;
    else
      zaddr_autoconfig <= 1'b0;
    
    // Z3 ------------------------------------------------
    z3addr2 <= {ZORRO_DATA_IN[15:8],ZORRO_ADDR_IN[22:1],2'b00};
    //z3addr2 <= {zD[15:8],zaddr[23:2],2'b00};
    
    // sample z3addr on falling edge of /FCS
    if (z3_fcs_state==0) begin
      if (znFCS_sync[0]==1 /*3'b111*/) begin
        z3_fcs_state <= 1;
        z3_end_cycle <= 1;
        z3addr <= 0; 
      end
    end else
      if (z3_fcs_state==1) begin
        if (znFCS_sync[0]==0 /*3'b000*/) begin // CHECK: if responding too quickly, this causes crashes
          z3_fcs_state <= 0;
          z3_end_cycle <= 0;
          z3addr <= z3addr2;
          //zorro_read  <= zREAD_sync[1];
          //zorro_write  <= ~zREAD_sync[1];
        end
      end
    
    if (z3_fcs_state==0) begin
      z3addr_in_ram <= (z3addr >= z3_ram_low) && (z3addr < z3_ram_high);
      z3addr_in_reg <= (z3addr >= z3_reg_low) && (z3addr < z3_reg_high);
    end else begin
      z3addr_in_ram <= 0;
      z3addr_in_reg <= 0;
    end
    
    z3addr_autoconfig <= (z3addr[31:16]=='hff00);
    
    z3_mapped_addr <= (z3addr-z3_ram_low)>>1;
    data_in_z3_low16 <= zaddr[23:8]; //zA[22:7]; // FIXME why sample this twice?
    
    if (znUDS_sync[1]==0 || znLDS_sync[1]==0 || znDS1_sync[1]==0 || znDS0_sync[1]==0)
      z3_din_latch <= 1;
    else
      z3_din_latch <= 0;
    
    // pipelined for better timing
    if (z3_din_latch) begin
      z3_din_high_s2 <= data_in; //zD;
      z3_din_low_s2  <= data_in_z3_low16; //zA[22:7];
    end
    
    // pipelined for better timing
    data_z3_hi16_latched <= data_z3_hi16;
    data_z3_low16_latched <= data_z3_low16;
    dataout_z3_latched <= dataout_z3;
    
    dtack_latched <= dtack;
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
  
  localparam RESET_DVID = 39;
  localparam COLD = 40;
  
  reg [7:0] zorro_state = COLD;
  reg zorro_idle = 0;
  reg [7:0] read_counter = 0;
  reg [7:0] dataout_time = 'h02;
  reg [7:0] datain_time = 'h10;
  reg [7:0] datain_counter = 0;
  reg [23:0] last_addr = 0;
  reg [23:0] last_read_addr = 0;
  reg [15:0] last_data = 0;
  reg [15:0] last_read_data = 0;
  reg z_confout = 0;
  reg [15:0] zaddr_regpart = 0;
  reg [15:0] z3addr_regpart = 0;
  reg [15:0] regread_addr = 0;
  reg [15:0] regwrite_addr = 0;
  
  reg [15:0] count_writes = 0;

  reg [15:0] testreg_1 = 0;
  
  always @(posedge S_AXI_ACLK) begin
    zorro_idle <= ((zorro_state==Z2_IDLE)||(zorro_state==Z3_IDLE));
    
    case (zorro_state)
      COLD: begin
        zorro_state <= RESET;
      end
      
      RESET: begin
        ram_low   <= 'h600000;
        ram_high  <= 'ha00000; //+ `RAM_SIZE-4;
        reg_low   <= 'h600000;
        reg_high  <= 'h60f000;
        
        dataout_enable <= 0;
        dataout <= 0;
        slaven <= 0;
        z_ovr <= 0;
        z_confout <= 0;
        z3_confdone <= 0;
        zorro_state <= DECIDE_Z2_Z3;
        
        count_writes <= 0;
      end
      
      DECIDE_Z2_Z3: begin
        ZORRO3 <= 0;
        zorro_state <= CONFIGURED;
      end
      
      Z2_CONFIGURING: begin
        z_ovr <= 0;
        if (zaddr_autoconfig && z_cfgin) begin
          if (z2_read) begin
            // read iospace 'he80000 (Autoconfig ROM)
            dataout_enable <= 1;
            dataout <= 1;
            slaven <= 1;
            
            case (zaddr_sync2[7:0])
              8'h00: data_out <= 'b1101_1111_1111_1111; // zorro 2 (11), no pool (0) rom (1)
              8'h02: data_out <= 'b0111_1111_1111_1111; // next board unrelated (0), 4mb (110 for 2mb)
              
              8'h04: data_out <= 'b1111_1111_1111_1111; // product number
              8'h06: data_out <= 'b1110_1111_1111_1111; // (1)
              
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
              case (zaddr_sync2[7:0])
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
      
      CONFIGURED: begin
        //ram_low <= ram_low + 'h10000;
        //ram_high <= ram_low + `RAM_SIZE;
        //reg_low <= ram_low;
        //reg_high <= ram_low + `REG_SIZE;
        
        zorro_state <= CONFIGURED_CLEAR;
      end
      
      CONFIGURED_CLEAR: begin
        zorro_state <= Z2_IDLE;
      end
      
      // ----------------------------------------------------------------------------------
      Z2_IDLE: begin
        if (z2_addr_valid) begin
          if (z2_read && zaddr_in_ram) begin
            // read RAM
            // request ram access from arbiter
            last_addr <= z2_mapped_addr;
            data_out <= default_data; //'hffff;
            read_counter <= 0;
            slaven <= 1;
            dataout_enable <= 1;
            dataout <= 1;
            z_ovr <= 1;
            zorro_state <= WAIT_READ3;
            
          end else if (z2_write && zaddr_in_ram) begin
            // write RAM
            last_addr <= z2_mapped_addr;
            dataout_enable <= 0;
            dataout <= 0;
            datain_counter <= 0;
            z_ovr <= 1;
            count_writes <= count_writes + 1;
            zorro_state <= WAIT_WRITE;
            
          end else if (z2_write && zaddr_in_reg) begin
            // write to register
            dataout_enable <= 0;
            dataout <= 0;
            z_ovr <= 1;
            zaddr_regpart <= zaddr_sync2[15:0];
            zorro_state <= Z2_REGWRITE;
            
          end else if (z2_read && zaddr_in_reg) begin
            // read from registers
            dataout_enable <= 1;
            dataout <= 1;
            data_out <= default_data; //'hffff;
            slaven <= 1;
            z_ovr <= 1;
            zaddr_regpart <= zaddr_sync2[15:0];
            zorro_state <= Z2_REGREAD;
            
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
      
      // ----------------------------------------------------------------------------------
      WAIT_READ3: begin
        zorro_ram_read_addr <= last_addr;
        zorro_ram_read_request <= 1;
        zorro_state <= WAIT_READ2;
      end
      
      WAIT_READ2: begin
        if (!zorro_ram_read_request && slv_reg0[30]==0) begin
          read_counter <= read_counter + 1;
          data_out <= slv_reg1; //zorro_ram_read_data;
          
          if (read_counter >= dataout_time) begin
            zorro_state <= Z2_ENDCYCLE;
          end
        end
      end
      
      WAIT_WRITE: begin
        if (!zorro_ram_write_request && slv_reg0[31]==0) begin
          if (datastrobe_synced) begin
            zorro_write_capture_bytes <= {~znUDS_sync[1],~znLDS_sync[1]};
            zorro_write_capture_data <= zdata_in_sync; //_sync;
            zorro_state <= WAIT_WRITE2;
          end
        end
      end
      
      WAIT_WRITE2: begin
        zorro_ram_write_addr <= last_addr;
        zorro_ram_write_bytes <= zorro_write_capture_bytes;
        zorro_ram_write_data <= zorro_write_capture_data;
        zorro_ram_write_request <= 1;
        zorro_state <= Z2_WRITE_FINALIZE;
      end
      
      Z2_WRITE_FINALIZE: begin
        if (!zorro_ram_write_request) begin
          zorro_state <= Z2_ENDCYCLE;
        end
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
      
      // FIXME why is there no dataout time on REGREAD? (see memory reads)
      // now fixed for Z3, still pending for Z2
      REGREAD: begin
        zorro_state <= Z2_REGREAD_POST;
        case (regread_addr&'hffc)
          'h00: begin
            rr_data[31:16] <= slv_reg0; end
          'h04: begin
            rr_data[31:16] <= testreg_1; end
          default: begin
            rr_data[31:16] <= REVISION;
            rr_data[15:0]  <= REVISION;
          end
        endcase
      end
      
      REGWRITE: begin
        zorro_state <= Z2_ENDCYCLE;
        
        case (regwrite_addr)
          'h00: testreg_1 <= regdata_in[15:0];
        endcase
      end
      
      default:
        // shouldn't happen
        zorro_state <= CONFIGURED;
      
    endcase
    
    if (slv_reg0[31]==1'b1) begin
      zorro_ram_write_request <= 0;
    end
    if (slv_reg0[30]==1'b1) begin
      zorro_ram_read_request <= 0;
    end
    
    out_reg0 <= last_addr;
    out_reg1 <= {16'h0000, zorro_write_capture_data};
    out_reg2 <= 0; // {ZORRO_NIORST,ZORRO_NFCS,ZORRO_NCCS,ZORRO_READ,ZORRO_DOE,ZORRO_NUDS,ZORRO_NLDS,ZORRO_NDS1,ZORRO_NDS0};
    out_reg3 <= {zorro_ram_write_request, zorro_ram_read_request, zorro_write_capture_bytes, 4'b0, 8'b0, 8'b0, zorro_state};
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
          2'h3   : reg_data_out <= out_reg3;
	      default : reg_data_out <= 'hdeadcafe;
	    endcase
	  end

endmodule
