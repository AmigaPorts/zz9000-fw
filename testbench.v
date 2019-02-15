`timescale 1ns/1ns

module testbench;
  reg aclk = 0;
  reg ready = 0;

  reg aresetn = 0;
      
  wire [31:0] s_axis_vid_tdata;
  wire s_axis_vid_tlast;
  reg s_axis_vid_tready;
  wire s_axis_vid_tuser;
  wire s_axis_vid_tvalid;
  reg s_axis_vid_aclk;
  
  video_tester vt(
                  .m_axis_vid_aclk(aclk),
                  .s_axis_vid_tdata(s_axis_vid_tdata),
                  .s_axis_vid_tlast(s_axis_vid_tlast),
                  .s_axis_vid_tready(ready),
                  .s_axis_vid_tuser(s_axis_vid_tuser),
                  .s_axis_vid_tvalid(s_axis_vid_tvalid)
                  );
  
  initial
    begin
      $dumpfile("testbench.vcd");
      $dumpvars(0,vt);
      
      ready = 1;
      aresetn = 1;
      //$display("testbench_state: %h %t",testbench_state,$time);

      #6000000 $finish;
    end

  always
    #5 aclk = !aclk;
  
endmodule
