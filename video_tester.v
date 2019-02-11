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
    reg valid = 0;
    reg ready = 1;
    reg start_of_frame = 0;
    reg eol = 0;
    reg [31:0] pixout;

    wire [7:0] red1;
    wire [7:0] green1;
    wire [7:0] blue1;
    
    wire [7:0] red2;
    wire [7:0] green2;
    wire [7:0] blue2;
     
    assign m_axis_vid_tready = count;
    assign s_axis_vid_tdata = pixout;
  
    assign red1 = {m_axis_vid_tdata[4:0],m_axis_vid_tdata[4:2]};
    assign green1 = {m_axis_vid_tdata[10:5], m_axis_vid_tdata[10:9]};
    assign blue1 = {m_axis_vid_tdata[15:11],m_axis_vid_tdata[15:13]};
    
    assign red2 = {m_axis_vid_tdata[20:16],m_axis_vid_tdata[20:18]};
    assign green2 = {m_axis_vid_tdata[26:21],m_axis_vid_tdata[26:25]};
    assign blue2 = {m_axis_vid_tdata[31:27],m_axis_vid_tdata[31:29] };
    
    assign s_axis_vid_tvalid = valid;
    assign s_axis_vid_tuser = start_of_frame;
    
    always @(posedge m_axis_vid_aclk) 
      if ((start_of_frame || valid) && s_axis_vid_tready)
        begin
          eol <= m_axis_vid_tlast;
          valid <= 1;
          case(count)    
            1: begin
              count <= 0;
              pixout  <= {red1,green1,blue1,8'b0};
             end
            0: begin
              count <= 1;
              pixout  <= {red2,green2,blue2,8'b0};
            end
          endcase
         end
      else if (eol)
        begin
          valid <= 0;
          ready <= 1;
          start_of_frame <= 0;
         end
      else if (m_axis_vid_tuser == 1 && s_axis_vid_tready)
        start_of_frame <= 1;
endmodule
