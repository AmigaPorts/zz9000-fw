`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/05/2019 07:38:44 PM
// Design Name: 
// Module Name: video_tester
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////

module video_tester(
  input [31:0] m_axis_vid_tdata,
  input m_axis_vid_tlast,
  output m_axis_vid_tready,
  input [0:0]  m_axis_vid_tuser,
  input m_axis_vid_tvalid,
  input m_axis_vid_aclk,
  input aresetn,
  
  output [31:0] s_axis_vid_tdata,
  output s_axis_vid_tlast,
  input s_axis_vid_tready,
  output [0:0] s_axis_vid_tuser,
  output s_axis_vid_tvalid,
  input s_axis_vid_aclk,
  
  output [15:0] dbg_x,
  output [15:0] dbg_y,
  output [2:0] dbg_state,
  output reg [15:0] dbg_pixcount,
  
  // control inputs for setting palette, width/height, scaling
  input [31:0] control_data,
  input [7:0] control_op
);

localparam OP_COLORMODE=1;
localparam OP_DIMENSIONS=2;
localparam OP_PALETTE=3;
localparam OP_SCALE=4;
localparam OP_VSYNC=5;

localparam CMODE_8BIT=0;
localparam CMODE_16BIT=1;
localparam CMODE_32BIT=2;
localparam CMODE_15BIT=4;

reg [15:0] screen_width = 640;
reg [15:0] screen_height = 480;
reg scale_x = 0;
reg scale_y = 0;
reg [31:0] palette[255:0];
reg [1:0] colormode = CMODE_16BIT;
reg vsync_request = 0;

localparam MAXWIDTH=1280;
reg [31:0] line_buffer[MAXWIDTH-1:0];

// (input) vdma state
reg [2:0] input_state = 0;
reg [9:0] inptr = 0;
reg ready_for_vdma = 0;

// (output) stream-to-video out state
reg [2:0] state = 0;
reg start_of_frame = 0;
reg ready;
reg valid = 0;
reg end_of_line = 0;
reg [15:0] cur_x = 0;
reg [15:0] cur_y = 0;

reg [31:0] pixout;
reg [7:0]  pixout8;
reg [15:0] pixout16;
reg [31:0] pixout32;
wire [7:0] red16   = {pixout16[4:0],   pixout16[4:2]};
wire [7:0] green16 = {pixout16[10:5],  pixout16[10:9]};
wire [7:0] blue16  = {pixout16[15:11], pixout16[15:13]};

assign s_axis_vid_tvalid = valid;
assign s_axis_vid_tuser  = start_of_frame;
assign s_axis_vid_tlast  = end_of_line;
assign s_axis_vid_tdata  = pixout;
assign m_axis_vid_tready = ready_for_vdma;

// TODO: logic to sync up the Y coordinate (line number)
//       for example, count vdma lines and if cur_y is not vdma_y,
//       wait until it is

reg [31:0] pixin;
reg pixin_valid = 0;
reg pixin_end_of_line = 0;
reg pixin_framestart = 0;

always @(posedge m_axis_vid_aclk)
  begin
    if (~aresetn) begin
      ready_for_vdma <= 0;
      input_state <= 0;
      inptr <= 0;
    end
    
    pixin <= m_axis_vid_tdata;
    pixin_valid <= m_axis_vid_tvalid;
    pixin_framestart <= m_axis_vid_tuser[0];
    pixin_end_of_line <= m_axis_vid_tlast;
    
    case (input_state)
      0: begin
          // wait for start of frame
          ready_for_vdma <= 1;
          inptr <= 0;
          if (pixin_framestart)
            input_state <= 3;
        end
      1: begin
          // reading from vdma
          ready_for_vdma <= 1;
        
          if (pixin_valid) begin
            line_buffer[inptr] <= pixin;
            
            if (pixin_end_of_line) begin
              inptr <= 0;
              input_state <= 2;
            end else if (inptr<screen_width) begin // FIXME we don't need to read so much
              inptr <= inptr + 1'b1;
            end else begin
              // done reading a line
              inptr <= 0;
              input_state <= 2;
            end
          end
        end
      2: begin
          // we've read more than enough of this line, wait until it's time for the next
          ready_for_vdma <= 0;
          
          if (vsync_request) input_state <= 0;
          
          // output line almost finished, time to read the next line
          if (cur_x >= screen_width-32) begin
            input_state <= 1;
          end
        end
      3: begin
          // we are at frame start, wait for the first line of video output
          ready_for_vdma <= 0;
          
          if (cur_y == 0) begin
            input_state <= 2;
          end
        end
    endcase
  end

assign dbg_state = state;
assign dbg_x = cur_x;
assign dbg_y = cur_y;

reg [31:0] control_data_in;
reg [7:0] control_op_in;

// control input
always @(posedge m_axis_vid_aclk)
begin

  control_op_in <= control_op;
  control_data_in <= control_data;
  
  case (control_op_in)
    OP_PALETTE: palette[control_data_in[31:24]] <= control_data_in[23:0];
    OP_DIMENSIONS: begin
        screen_height <= control_data_in[31:16];
        screen_width  <= control_data_in[15:0];
      end
    OP_SCALE: begin
        scale_x  <= control_data_in[0];
        scale_y  <= control_data_in[1];
      end
    OP_COLORMODE: colormode  <= control_data_in[1:0];
    OP_VSYNC: vsync_request <= control_data[0];
  endcase
end

wire [9:0] cur_x_linebuf = colormode==CMODE_32BIT ? cur_x : (colormode==CMODE_16BIT ? cur_x[9:1] : cur_x[9:2]);
reg [31:0] palout;

always @(posedge m_axis_vid_aclk)
begin

  case (cur_x[1:0])
    2'b11: pixout8 <= pixout32[31:24];
    2'b10: pixout8 <= pixout32[23:16];
    2'b01: pixout8 <= pixout32[15:8];
    2'b00: pixout8 <= pixout32[7:0];
  endcase
  
  case (cur_x[0])
    1'b1: pixout16 <= {pixout32[23:16],pixout32[31:24]};
    1'b0: pixout16 <= {pixout32[7:0],  pixout32[15:8]};
  endcase
  
  pixout32 <= line_buffer[cur_x_linebuf];
  palout <= palette[pixout8];
  
  case (colormode)
    CMODE_16BIT: pixout <= {8'b0,blue16,green16,red16};
    CMODE_8BIT:  pixout <= palout;
    CMODE_32BIT: pixout <= pixout32;
  endcase
  
  ready <= s_axis_vid_tready;
  
  if (~aresetn) begin
    // reset or VDMA frame start not reached
    cur_x <= 0;
    cur_y <= 0;
    state <= 0;
    valid <= 0;
    start_of_frame <= 0;
    end_of_line <= 0;
  end
  else begin
    
    // video out control
    if (ready) begin
      valid <= 1;
      
      if (cur_x >= screen_width-1) begin
        // end of line
        cur_x <= 0;
        end_of_line <= 1;
        
        if (cur_y >= screen_height-1) begin
          cur_y <= 0;
        end
        else begin
          cur_y <= cur_y + 1'b1;
        end
      end
      else begin
        // next pixel
        cur_x <= cur_x + 1'b1;
        end_of_line <= 0;
        
        // start of frame
        if (cur_x==0 && cur_y==0) 
          start_of_frame <= 1;
        else
          start_of_frame <= 0;
      end
    end
  end
end
  
  
endmodule
