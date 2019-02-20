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
  output reg [15:0] dbg_pixcount
);

localparam WIDTH=640;
localparam HEIGHT=480;
reg [31:0] line_buffer[(2*WIDTH-1):0];

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
reg [15:0] pixout16;
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

always @(posedge m_axis_vid_aclk)
  begin
    if (~aresetn) begin
      ready_for_vdma <= 0;
      input_state <= 0;
      inptr <= 0;
    end
    else if (input_state == 0) begin
      // wait for start of frame
      if (m_axis_vid_tuser)
        input_state <= 1;
    end
    else if (input_state == 1) begin
      // reading from vdma
      ready_for_vdma <= 1;
    
      if (m_axis_vid_tvalid) begin
        line_buffer[inptr] <= m_axis_vid_tdata;
        
        if (m_axis_vid_tlast) begin
          inptr <= 0;
          input_state <= 2;
        end else if (inptr<WIDTH) begin
          inptr <= inptr + 1'b1;
        end else begin
          // done reading a line
          inptr <= 0;
          input_state <= 2;
        end
      end
    end else if (input_state == 2) begin
      // read more than enough
      ready_for_vdma <= 0;
      
      // output line almost finished, time to read the next line
      if (cur_x >= WIDTH-32) begin
        input_state <= 1;
      end
    end
  end

assign dbg_state = state;
assign dbg_x = cur_x;
assign dbg_y = cur_y;

always @(posedge m_axis_vid_aclk)
begin

  if (cur_x[0]==1'b1)
    pixout16 <= {line_buffer[cur_x[9:1]][23:16],line_buffer[cur_x[9:1]][31:24]};
  else
    pixout16 <= {line_buffer[cur_x[9:1]][7:0],  line_buffer[cur_x[9:1]][15:8]};
  
  pixout <= {8'b0,blue16,green16,red16};
  
  //pixout <= {cur_y,cur_x};
  ready <= s_axis_vid_tready;
  
  if (valid && ready) begin
    if (end_of_line) begin
      //$display($time, " end_of_line: %d",dbg_pixcount);
      dbg_pixcount <= 0;
    end else
      dbg_pixcount <= dbg_pixcount + 1'b1;
  end
  
  if (~aresetn || input_state==0) begin
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
      
      if (cur_x >= WIDTH-1) begin
        // end of line
        cur_x <= 0;
        end_of_line <= 1;
        
        if (cur_y >= HEIGHT-1) begin
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
