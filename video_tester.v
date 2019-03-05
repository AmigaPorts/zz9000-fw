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
  
  input dvi_clk,
  output reg dvi_hsync,
  output reg dvi_vsync,
  output reg dvi_active_video,
  output reg [31:0] dvi_rgb,
  
  // control inputs for setting palette, width/height, scaling
  input [31:0] control_data,
  input [7:0] control_op,
  output reg [7:0] dbg_state
);

localparam OP_COLORMODE=1;
localparam OP_DIMENSIONS=2;
localparam OP_PALETTE=3;
localparam OP_SCALE=4;
localparam OP_VSYNC=5;
localparam OP_MAX=6;
localparam OP_HS=7;
localparam OP_VS=8;
localparam OP_THRESH=9;

localparam CMODE_8BIT=0;
localparam CMODE_16BIT=1;
localparam CMODE_32BIT=2;
localparam CMODE_15BIT=4;

reg [15:0] screen_width = 1280;
reg [15:0] screen_height = 720;
reg scale_x = 0;
reg scale_y = 0; // amiga boots in 640x256, so double the resolution vertically
reg [31:0] palette[255:0];
reg [2:0] colormode = CMODE_32BIT;
reg vsync_request = 0;
reg [3:0] div_x = 0;
reg [15:0] fetch_threshold = 2;

reg [15:0] screen_h_max = 1980;
reg [15:0] screen_v_max = 750;
reg [15:0] screen_h_sync_start = 1720;
reg [15:0] screen_h_sync_end = 1760;
reg [15:0] screen_v_sync_start = 725;
reg [15:0] screen_v_sync_end = 730;

localparam MAXWIDTH=1280;
reg [31:0] line_buffer[MAXWIDTH-1:0];

// (input) vdma state
reg [3:0] input_state = 0;
reg [3:0] next_input_state = 0;
reg [15:0] inptr = 0;
reg ready_for_vdma = 0;

// (output) stream-to-video out state
reg [2:0] state = 0;
reg start_of_frame = 0;
reg ready;
reg valid = 0;
reg end_of_line = 0;
reg [15:0] cur_x = 0;
reg [15:0] cur_y = 0;

assign m_axis_vid_tready = ready_for_vdma;

reg [15:0] counter_x = 0;
reg [15:0] counter_x_dly = 0;
reg [15:0] counter_y = 0;

reg [31:0] pixin;
reg pixin_valid = 0;
reg pixin_end_of_line = 0;
reg pixin_framestart = 0;
reg [15:0] need_line_fetch = 0;
reg [15:0] need_line_fetch_reg = 0;
reg [15:0] need_line_fetch_reg2 = 0;
reg [15:0] last_line_fetch = 1;
reg [15:0] screen_width_shifted = 0;
reg [15:0] input_line = 0;
reg [15:0] screen_height_cmp = 0;

always @(posedge m_axis_vid_aclk)
  begin
    if (~aresetn) begin
      ready_for_vdma <= 0;
      next_input_state <= 0;
      inptr <= 0;
    end
    
    dbg_state <= input_state;
    input_state <= next_input_state;
    need_line_fetch_reg <= need_line_fetch;
    need_line_fetch_reg2 <= need_line_fetch_reg>>scale_y; // line duplication
    
    cur_x <= counter_x;
    cur_y <= counter_y;
    
    case (colormode)
      0: div_x <= (4'b0010);
      1: div_x <= (4'b0001);
      2: div_x <= (4'b0000);
    endcase
    screen_width_shifted <= (screen_width>>div_x)>>scale_x;
    
    pixin <= m_axis_vid_tdata;
    pixin_valid <= m_axis_vid_tvalid;
    pixin_framestart <= m_axis_vid_tuser[0];
    pixin_end_of_line <= m_axis_vid_tlast;
    
    case (input_state)
      4'h0: begin
          // wait for start of frame
          ready_for_vdma <= 1;
          inptr <= 0;
          input_line <= 0;
          if (pixin_framestart)
            next_input_state <= 4'h3;
        end
      4'h1: begin
          // reading from vdma
          ready_for_vdma <= 1;
          last_line_fetch <= need_line_fetch_reg2;
        
          if (pixin_valid) begin
            line_buffer[inptr] <= pixin;
            
            if (pixin_end_of_line) begin
              inptr <= 0;
              next_input_state <= 4'h2;
            end else if (inptr<screen_width_shifted) begin // FIXME we don't need to read so much
              inptr <= inptr + 1'b1;
            end
          end
        end
      4'h2: begin
          // we've read more than enough of this line, wait until it's time for the next
          ready_for_vdma <= 0;
          
          if (vsync_request) next_input_state <= 4'h0;
          else 
          if (need_line_fetch_reg2!=last_line_fetch) begin
            // time to read the next line
            next_input_state <= 4'h1;
          end
        end
      4'h3: begin
          // we are at frame start, wait for the first line of video output
          ready_for_vdma <= 0;
          inptr <= 0;
          
          if (need_line_fetch_reg2 == 0) begin
            next_input_state <= 4'h2;
          end
        end
    endcase
  end

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
    OP_MAX: begin
        screen_v_max <= control_data_in[31:16];
        screen_h_max <= control_data_in[15:0];
      end
    OP_HS: begin
        screen_h_sync_start <= control_data_in[31:16];
        screen_h_sync_end <= control_data_in[15:0];
      end
    OP_VS: begin
        screen_v_sync_start <= control_data_in[31:16];
        screen_v_sync_end <= control_data_in[15:0];
      end
    OP_THRESH: begin
        fetch_threshold <= control_data_in[15:0];
      end
  endcase
end

reg [31:0] palout;
reg vga_reset = 0;
reg [15:0] vga_v_rez = 720;
reg [15:0] vga_h_rez = 1280;
reg [15:0] vga_v_max = 750;
reg [15:0] vga_h_max = 1980;
reg [15:0] vga_h_sync_start = 1720;
reg [15:0] vga_h_sync_end = 1760;
reg [15:0] vga_v_sync_start = 725;
reg [15:0] vga_v_sync_end = 730;
reg [15:0] counter_scanout = 0;
reg display_pixels = 0;
reg [2:0] vga_colormode = 2;

reg vga_scale_x = 0;
reg [31:0] pixout;
reg [7:0]  pixout8;
reg [15:0] pixout16;
reg [31:0] pixout32;
wire [7:0] red16   = {pixout16[4:0],   pixout16[4:2]};
wire [7:0] green16 = {pixout16[10:5],  pixout16[10:9]};
wire [7:0] blue16  = {pixout16[15:11], pixout16[15:13]};

wire [15:0] cur_x_linebuf = vga_colormode==CMODE_32BIT ? (counter_x>>vga_scale_x) 
                                                       : (vga_colormode==CMODE_16BIT ? (counter_x>>(2'b01+vga_scale_x)) 
                                                                                     : (counter_x>>(2'b10+vga_scale_x)));

reg [7:0] vga_fetch_threshold = 0;
reg [15:0] vga_w2 = 0;

always @(posedge dvi_clk) begin
  vga_h_rez <= screen_width;
  vga_v_rez <= screen_height;
  vga_h_max <= screen_h_max;
  vga_v_max <= screen_v_max;
  vga_h_sync_start <= screen_h_sync_start;
  vga_h_sync_end <= screen_h_sync_end;
  vga_v_sync_start <= screen_v_sync_start;
  vga_v_sync_end <= screen_v_sync_end;
  vga_scale_x <= scale_x;
  vga_colormode <= colormode;
  vga_fetch_threshold <= fetch_threshold;
  
  counter_x_dly <= counter_x;
  
  if (vga_scale_x==1) begin
    case (counter_x_dly[2:1])
      2'b11: pixout8 <= pixout32[31:24];
      2'b10: pixout8 <= pixout32[23:16];
      2'b01: pixout8 <= pixout32[15:8];
      2'b00: pixout8 <= pixout32[7:0];
    endcase
  end else begin
    case (counter_x_dly[1:0])
      2'b11: pixout8 <= pixout32[31:24];
      2'b10: pixout8 <= pixout32[23:16];
      2'b01: pixout8 <= pixout32[15:8];
      2'b00: pixout8 <= pixout32[7:0];
    endcase
  end
  
  if (vga_scale_x==1) begin
    case (counter_x_dly[1])
      1'b1: pixout16 <= {pixout32[23:16],pixout32[31:24]};
      1'b0: pixout16 <= {pixout32[7:0]  ,pixout32[15:8] };
    endcase
  end else begin
    case (counter_x_dly[0])
      1'b1: pixout16 <= {pixout32[23:16],pixout32[31:24]};
      1'b0: pixout16 <= {pixout32[7:0]  ,pixout32[15:8] };
    endcase
  end
  
  pixout32 <= line_buffer[cur_x_linebuf];
  palout <= palette[pixout8];
  
  case (vga_colormode)
    CMODE_16BIT: pixout <= {8'b0,blue16,green16,red16};
    CMODE_8BIT:  pixout <= palout;
    CMODE_32BIT: pixout <= pixout32;
  endcase
  
  dvi_rgb <= pixout;
  
  if (vga_reset) begin
    counter_x <= 0;
    counter_y <= vga_v_max;
  end else if (counter_x > vga_h_max) begin
    counter_x <= 0;
    if (counter_y > vga_v_max) begin
      counter_y <= 0;
    end else begin
      counter_y <= counter_y + 1'b1;
    end
  end else begin
    counter_x <= counter_x + 1'b1;
  end
  
  if (counter_y<vga_v_rez-1) begin
    if (counter_x>vga_h_rez-16)  // FIXME was fetch_threshold
      need_line_fetch <= counter_y + 1'b1;
  end else
    need_line_fetch <= 0;
  
  if (counter_x>=vga_h_sync_start && counter_x<vga_h_sync_end)
    dvi_hsync <= 1;
  else
    dvi_hsync <= 0;
    
  if (counter_y>=vga_v_sync_start && counter_y<vga_v_sync_end)
    dvi_vsync <= 1;
  else
    dvi_vsync <= 0;
  
  vga_w2 <= (vga_h_rez+vga_fetch_threshold);
  
  if (counter_x>vga_fetch_threshold && counter_x<vga_w2 && counter_y<vga_v_rez) begin
    dvi_active_video <= 1;
  end else begin
    dvi_active_video <= 0;
  end
end  

endmodule