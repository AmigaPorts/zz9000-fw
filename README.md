# MNT ZZ9000 ZYNQ FPGA Code and ZZ9000OS C/ARM Code

MNT ZZ9000 is a graphics and ARM coprocessor card for Amiga computers equipped with Zorro slots. It is based on a Xilinx ZYNQ Z-7020 chip that combines 7-series FPGA fabric with dual ARM Cortex-A9 CPUs clocked at 666MHz. The current version has 1GB of DDR3 RAM and no eMMC soldered.

This repository contains the sources for the FPGA logic as well as the miniature "operating system" that runs on the ARM core 0 and instruments core 1 for user applications.

Warning: this is still in the process of cleanup. Many hacks have been made.

# Requirements

Currently requires Xilinx Vivado Webpack version.

# Getting around the Code

The interesting bits:

- `mntzorro.v` is the Zorro 2/3 interface and 24-bit video capture engine, AXI4-Lite interface
- `video_formatter.v` is the AXI video stream formatter that reinterprets and controls an incoming 32-bit word stream as 8-bit palette indexed, 16-bit 565 or 24-bit RGBX pixels and outputs a 24-bit true color parallel RGB stream with horizontal and vertical sync
- `zz9000-fw/ZZ9000_proto.sdk/ZZ9000Test/src`
  - `main.c` is the main entrypoint of ZZ9000OS.elf which runs on the ARM core 0 and talks to `MNTZorro` and `video_formatter` in the FPGA fabric
  - `gfx.c` graphics acceleration routines, currently mainly rect fill and copy
  - `ethernet.c` low-level ethernet driver/framer
- `ZZ9000_proto.srcs/constrs_1/new/zz9000.xdc` XDC constraints file that contains the pin/ball mappings and some timing voodoo

![ZZ9000 Block Design](gfx/zz9000-bd.png?raw=true "ZZ9000 Block Design")

# Set up Project

As Vivado projects are not suitable for version control, the Vivado project / block design is exported as a TCL script zz9000_ps.tcl. Apparently you can start vivado in TCL mode and then source this file to recreate the project incl. block design. I'm not sure yet how to automatically rebuild the SDK (Eclipse) project incl. FSBL, but all relevant sources are already in this repo.

# Build Result

The Eclipse-based Vivado SDK has a built-in tool (Xilinx / Generate Boot Image) to generate the boot image `BOOT.bin` from the definition file `ZZ9000_proto.sdk/ZZ9000Test/bootimage/ZZ9000OS.bif`.

`BOOT.bin` contains 3 files (“partitions”):

- `FSBL.elf` (first stage bootloader, written in C, Xilinx default)
- FPGA bitfile (synthesized by Vivado from Verilog sources and Block Design)
- `ZZ9000OS.elf` (ZZ9000OS ARM executable, written in C)

# Hardware Connectivity

Schematics are in the manual (PDF): https://mntre.com/media/ZZ9000_info_md/zz9000-manual.pdf

- DVI (via non-HDMI compliant HDMI connector) with SiliconImage 9022 encoder
- Gigabit Ethernet port with Micrel KSZ9031 PHY
- MicroSD slot (for firmware loading)
- USB 2.0 port (not yet used)

# License / Copyright

If not stated otherwise in specific source code files, everything here is:

Copyright (C) 2016-2019, Lukas F. Hartmann <lukas@mntre.com>
MNT Research GmbH, Berlin
https://mntre.com

SPDX-License-Identifier: GPL-3.0-or-later
https://spdx.org/licenses/GPL-3.0-or-later.html

# Making the Platform Cable Work

First, `apt install fxload`

Copy the `xilinx-xusb` folder to `/etc/xilinx-xusb`.

Edit `/etc/udev/rules.d/xusbdfwu.rules` with the following content:

```
# version 0003
ATTRS{idVendor}=="03fd", ATTRS{idProduct}=="0008", MODE="666"
SUBSYSTEM=="usb", ACTION=="add", ATTRS{idVendor}=="03fd", ATTRS{idProduct}=="0007", RUN+="/sbin/fxload -v -t fx2 -I /etc/xilinx-xusb/xusbdfwu.hex -D $tempnode"
SUBSYSTEM=="usb", ACTION=="add", ATTRS{idVendor}=="03fd", ATTRS{idProduct}=="0009", RUN+="/sbin/fxload -v -t fx2 -I /etc/xilinx-xusb/xusb_xup.hex -D $tempnode"
SUBSYSTEM=="usb", ACTION=="add", ATTRS{idVendor}=="03fd", ATTRS{idProduct}=="000d", RUN+="/sbin/fxload -v -t fx2 -I /etc/xilinx-xusb/xusb_emb.hex -D $tempnode"
SUBSYSTEM=="usb", ACTION=="add", ATTRS{idVendor}=="03fd", ATTRS{idProduct}=="000f", RUN+="/sbin/fxload -v -t fx2 -I /etc/xilinx-xusb/xusb_xlp.hex -D $tempnode"
SUBSYSTEM=="usb", ACTION=="add", ATTRS{idVendor}=="03fd", ATTRS{idProduct}=="0013", RUN+="/sbin/fxload -v -t fx2 -I /etc/xilinx-xusb/xusb_xp2.hex -D $tempnode"
SUBSYSTEM=="usb", ACTION=="add", ATTRS{idVendor}=="03fd", ATTRS{idProduct}=="0015", RUN+="/sbin/fxload -v -t fx2 -I /etc/xilinx-xusb/xusb_xse.hex -D $tempnode"
```

This will make udev launch fxload whenever the platform cable is plugged in. Fxload will in turn load those hex files (firmware) into the cable. The LED on the cable should then turn green.

