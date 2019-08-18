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

  reg [15:0] col_count = 0;
  reg [15:0] last_col_count = 0;
  
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
      
      ready = 0;
      aresetn = 1;
      //$display("testbench_state: %h %t",testbench_state,$time);

      #1010 ready = 0;
      #1000 ready = 1;
      #1000000 ready = 0;
      #1000000 ready = 1;
      #100 ready = 0;
      #20 ready = 1;
      #20 ready = 0;
      #20 ready = 1;
      #20 ready = 0;
      #20 ready = 1;
      #20 ready = 0;
      #20 ready = 1;
      
      #100000000 $finish;
    end

  always
    begin
      #10 aclk = !aclk;
      if (aclk) begin
        if (s_axis_vid_tvalid && ready) begin
          col_count = col_count + 1;
          if (s_axis_vid_tlast) begin
            col_count = 0;
          end
        end
      end
    end
  
endmodule
