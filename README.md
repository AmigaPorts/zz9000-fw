# MNT ZZ9000 ZYNQ FPGA Code and ZZ9000OS C/ARM Code

MNT ZZ9000 is a graphics and ARM coprocessor card for Amiga computers equipped with Zorro slots. It is based on a Xilinx ZYNQ Z-7020 chip that combines 7-series FPGA fabric with dual ARM Cortex-A9 CPUs clocked at 666MHz. The current version has 1GB of DDR3 RAM and no eMMC soldered.

This repository contains the sources for the FPGA logic as well as the miniature "operating system" that runs on the ARM core 0 and instruments core 1 for user applications.

Warning: this is still in the process of cleanup. Many hacks have been made.

# Requirements

Currently requires Xilinx Vivado Webpack version.

# Set up Project

As Vivado projects are not suitable for version control, the Vivado project / block design is exported as a TCL script zz9000-project.tcl. Apparently you can start vivado in TCL mode and then source this file to recreate the project incl. block design.

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

