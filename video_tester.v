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
  input m_axis_vid_tuser,
  input m_axis_vid_tvalid,
  input m_axis_vid_aclk,
  
  output [31:0] s_axis_vid_tdata,
  output s_axis_vid_tlast,
  input s_axis_vid_tready,
  output s_axis_vid_tuser,
  output s_axis_vid_tvalid,
  input s_axis_vid_aclk
);

reg count = 1;
//reg valid = 0;
//reg ready = 1;
//reg start_of_frame = 0;
//reg eol = 0;
reg [31:0] pixbuf1[(2*640-1):0];

reg [31:0] pixout;
reg [31:0] pixout1;
reg [31:0] pixout2;
reg [9:0] inptr=0;
reg [9:0] outptr=0;

wire [7:0] red1;
wire [7:0] green1;
wire [7:0] blue1;
  
wire [7:0] red2;
wire [7:0] green2;
wire [7:0] blue2;

reg valid = 0;

assign s_axis_vid_tdata = pixout;
assign m_axis_vid_tready = s_axis_vid_tready;

assign red1 = {pixout1[4:0], pixout1[4:2]};
assign green1 = {pixout1[10:5], pixout1[10:9]};
assign blue1 = {pixout1[15:11], pixout1[15:13]};

assign red2 = {pixout1[20:16], pixout1[20:18]};
assign green2 = {pixout1[26:21], pixout1[26:25]};
assign blue2 = {pixout1[31:27], pixout1[31:29]};

reg [31:0] pixin1;

//assign s_axis_vid_tvalid = valid;
//assign s_axis_vid_tuser = start_of_frame;
assign s_axis_vid_tvalid = m_axis_vid_tvalid;
assign s_axis_vid_tuser = m_axis_vid_tuser;
assign s_axis_vid_tlast = m_axis_vid_tlast;

reg prev_tlast = 0;
reg [1:0] state = 0;
reg tlast_count = 0;

always @(posedge m_axis_vid_aclk)
  begin
    valid <= s_axis_vid_tready;
    
    prev_tlast <= m_axis_vid_tlast;
    
    // line end
    if (m_axis_vid_tlast==1 && prev_tlast==0 && state==0) begin
      tlast_count <= tlast_count+1;
      
      count <= 0;
      inptr <= 0;
      outptr <= 0;
      state <= 1;
    end
    
    if (state==1) begin
      if (m_axis_vid_tvalid)
        state <= 2;
    end
    
    if (state==2) begin
      count <= ~count;
      pixbuf1[inptr] <= pixin1;
      if (inptr<640) begin
        inptr<=inptr+1;
      end
        
      if (count)
        pixout <= {8'b0,blue1,green1,red1};
      else
        pixout <= {8'b0,blue2,green2,red2};
      
      if (count) begin
        if (outptr<640) outptr<=outptr+1;
        else state <= 0;
      end
    end
    
    pixin1 <= m_axis_vid_tdata;
    pixout1 <= {pixbuf1[outptr][23:16],pixbuf1[outptr][31:24],pixbuf1[outptr][7:0],pixbuf1[outptr][15:8]};
    
    // buffer a word
    // tvalid is somehow too laggy, can't wait for it
    
  end
endmodule
