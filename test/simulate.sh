set -e
iverilog -DSIMULATION=1 -otestbench testbench.v ./video_formatter.v
./testbench
