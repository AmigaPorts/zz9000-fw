`timescale 1ns/1ns

module testbench;
  reg aclk = 0;
  reg dvi_clk = 0;
  reg ready = 0;
  reg aresetn = 0;
  reg [31:0] control_data = 0;
  reg [7:0] control_op = 0;
  
  video_formatter vf(
                  .m_axis_vid_aclk(aclk),
                  .dvi_clk(dvi_clk),
                  .dvi_hsync(dvi_hsync),
                  .dvi_vsync(dvi_vsync),
                  .dvi_active_video(dvi_active_video),
                  .control_data(control_data),
                  .control_op(control_op)
                  );
  
  initial
    begin
      $dumpfile("testbench.vcd");
      $dumpvars(0,vf);
      
      ready = 0;
      aresetn = 1;

      control_data <= 1;
      control_op <= 11; // OP_RESET
      #10 control_op <= 0;
      
      #100000 $finish;
    end

  always
    begin
      #2 aclk = !aclk;
      #2 dvi_clk = !dvi_clk;
    end
  
endmodule
