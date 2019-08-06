
################################################################
# This is a generated script based on design: zz9000_ps
#
# Though there are limitations about the generated script,
# the main purpose of this utility is to make learning
# IP Integrator Tcl commands easier.
################################################################

namespace eval _tcl {
proc get_script_folder {} {
   set script_path [file normalize [info script]]
   set script_folder [file dirname $script_path]
   return $script_folder
}
}
variable script_folder
set script_folder [_tcl::get_script_folder]

################################################################
# Check if script is running in correct Vivado version.
################################################################
set scripts_vivado_version 2018.3
set current_vivado_version [version -short]

if { [string first $scripts_vivado_version $current_vivado_version] == -1 } {
   puts ""
   catch {common::send_msg_id "BD_TCL-109" "ERROR" "This script was generated using Vivado <$scripts_vivado_version> and is being run in <$current_vivado_version> of Vivado. Please run the script in Vivado <$scripts_vivado_version> then open the design in Vivado <$current_vivado_version>. Upgrade the design by running \"Tools => Report => Report IP Status...\", then run write_bd_tcl to create an updated script."}

   return 1
}

################################################################
# START
################################################################

# To test this script, run the following commands from Vivado Tcl console:
# source zz9000_ps_script.tcl


# The design that will be created by this Tcl script contains the following 
# module references:
# MNTZorro_v0_1_S00_AXI, video_tester

# Please add the sources of those modules before sourcing this Tcl script.

# If there is no project opened, this script will create a
# project, but make sure you do not have an existing project
# <./myproj/project_1.xpr> in the current working folder.

set list_projs [get_projects -quiet]
if { $list_projs eq "" } {
   create_project project_1 myproj -part xc7z020clg400-1
}


# CHANGE DESIGN NAME HERE
variable design_name
set design_name zz9000_ps

# If you do not already have an existing IP Integrator design open,
# you can create a design using the following command:
#    create_bd_design $design_name

# Creating design if needed
set errMsg ""
set nRet 0

set cur_design [current_bd_design -quiet]
set list_cells [get_bd_cells -quiet]

if { ${design_name} eq "" } {
   # USE CASES:
   #    1) Design_name not set

   set errMsg "Please set the variable <design_name> to a non-empty value."
   set nRet 1

} elseif { ${cur_design} ne "" && ${list_cells} eq "" } {
   # USE CASES:
   #    2): Current design opened AND is empty AND names same.
   #    3): Current design opened AND is empty AND names diff; design_name NOT in project.
   #    4): Current design opened AND is empty AND names diff; design_name exists in project.

   if { $cur_design ne $design_name } {
      common::send_msg_id "BD_TCL-001" "INFO" "Changing value of <design_name> from <$design_name> to <$cur_design> since current design is empty."
      set design_name [get_property NAME $cur_design]
   }
   common::send_msg_id "BD_TCL-002" "INFO" "Constructing design in IPI design <$cur_design>..."

} elseif { ${cur_design} ne "" && $list_cells ne "" && $cur_design eq $design_name } {
   # USE CASES:
   #    5) Current design opened AND has components AND same names.

   set errMsg "Design <$design_name> already exists in your project, please set the variable <design_name> to another value."
   set nRet 1
} elseif { [get_files -quiet ${design_name}.bd] ne "" } {
   # USE CASES: 
   #    6) Current opened design, has components, but diff names, design_name exists in project.
   #    7) No opened design, design_name exists in project.

   set errMsg "Design <$design_name> already exists in your project, please set the variable <design_name> to another value."
   set nRet 2

} else {
   # USE CASES:
   #    8) No opened design, design_name not in project.
   #    9) Current opened design, has components, but diff names, design_name not in project.

   common::send_msg_id "BD_TCL-003" "INFO" "Currently there is no design <$design_name> in project, so creating one..."

   create_bd_design $design_name

   common::send_msg_id "BD_TCL-004" "INFO" "Making design <$design_name> as current_bd_design."
   current_bd_design $design_name

}

common::send_msg_id "BD_TCL-005" "INFO" "Currently the variable <design_name> is equal to \"$design_name\"."

if { $nRet != 0 } {
   catch {common::send_msg_id "BD_TCL-114" "ERROR" $errMsg}
   return $nRet
}

set bCheckIPsPassed 1
##################################################################
# CHECK IPs
##################################################################
set bCheckIPs 1
if { $bCheckIPs == 1 } {
   set list_check_ips "\ 
xilinx.com:ip:axi_protocol_converter:2.1\
xilinx.com:ip:axi_register_slice:2.1\
xilinx.com:ip:clk_wiz:6.0\
xilinx.com:ip:proc_sys_reset:5.0\
xilinx.com:ip:processing_system7:5.5\
xilinx.com:ip:xlslice:1.0\
xilinx.com:ip:axi_vdma:6.3\
xilinx.com:ip:axis_data_fifo:2.0\
"

   set list_ips_missing ""
   common::send_msg_id "BD_TCL-006" "INFO" "Checking if the following IPs exist in the project's IP catalog: $list_check_ips ."

   foreach ip_vlnv $list_check_ips {
      set ip_obj [get_ipdefs -all $ip_vlnv]
      if { $ip_obj eq "" } {
         lappend list_ips_missing $ip_vlnv
      }
   }

   if { $list_ips_missing ne "" } {
      catch {common::send_msg_id "BD_TCL-115" "ERROR" "The following IPs are not found in the IP Catalog:\n  $list_ips_missing\n\nResolution: Please add the repository containing the IP(s) to the project." }
      set bCheckIPsPassed 0
   }

}

##################################################################
# CHECK Modules
##################################################################
set bCheckModules 1
if { $bCheckModules == 1 } {
   set list_check_mods "\ 
MNTZorro_v0_1_S00_AXI\
video_tester\
"

   set list_mods_missing ""
   common::send_msg_id "BD_TCL-006" "INFO" "Checking if the following modules exist in the project's sources: $list_check_mods ."

   foreach mod_vlnv $list_check_mods {
      if { [can_resolve_reference $mod_vlnv] == 0 } {
         lappend list_mods_missing $mod_vlnv
      }
   }

   if { $list_mods_missing ne "" } {
      catch {common::send_msg_id "BD_TCL-115" "ERROR" "The following module(s) are not found in the project: $list_mods_missing" }
      common::send_msg_id "BD_TCL-008" "INFO" "Please add source files for the missing module(s) above."
      set bCheckIPsPassed 0
   }
}

if { $bCheckIPsPassed != 1 } {
  common::send_msg_id "BD_TCL-1003" "WARNING" "Will not continue with creation of design due to the error(s) above."
  return 3
}

##################################################################
# DESIGN PROCs
##################################################################


# Hierarchical cell: video_subsystem
proc create_hier_cell_video_subsystem { parentCell nameHier } {

  variable script_folder

  if { $parentCell eq "" || $nameHier eq "" } {
     catch {common::send_msg_id "BD_TCL-102" "ERROR" "create_hier_cell_video_subsystem() - Empty argument(s)!"}
     return
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_msg_id "BD_TCL-100" "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_msg_id "BD_TCL-101" "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj

  # Create cell and set as current instance
  set hier_obj [create_bd_cell -type hier $nameHier]
  current_bd_instance $hier_obj

  # Create interface pins
  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 M_AXI_MM2S
  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 S_AXI_LITE1

  # Create pins
  create_bd_pin -dir O VGA_DE
  create_bd_pin -dir O VGA_HS
  create_bd_pin -dir O VGA_VS
  create_bd_pin -dir I -type rst axi_resetn
  create_bd_pin -dir I -type rst axi_resetn1
  create_bd_pin -dir I -type clk clk
  create_bd_pin -dir I -from 31 -to 0 control_data
  create_bd_pin -dir I control_interlace
  create_bd_pin -dir I -from 7 -to 0 control_op
  create_bd_pin -dir O -from 7 -to 0 dbg_state
  create_bd_pin -dir I -type clk s_axi_aclk
  create_bd_pin -dir I -type clk s_axi_lite_aclk
  create_bd_pin -dir O -from 31 -to 0 vid_data

  # Create instance: axi_vdma_0, and set properties
  set axi_vdma_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_vdma:6.3 axi_vdma_0 ]
  set_property -dict [ list \
   CONFIG.c_include_mm2s_dre {0} \
   CONFIG.c_include_s2mm {0} \
   CONFIG.c_m_axi_mm2s_data_width {32} \
   CONFIG.c_mm2s_genlock_mode {0} \
   CONFIG.c_mm2s_linebuffer_depth {2048} \
   CONFIG.c_mm2s_max_burst_length {128} \
   CONFIG.c_num_fstores {1} \
   CONFIG.c_s2mm_genlock_mode {0} \
 ] $axi_vdma_0

  # Create instance: axis_data_fifo_0, and set properties
  set axis_data_fifo_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_data_fifo:2.0 axis_data_fifo_0 ]
  set_property -dict [ list \
   CONFIG.FIFO_DEPTH {32} \
 ] $axis_data_fifo_0

  # Create instance: video_tester_0, and set properties
  set block_name video_tester
  set block_cell_name video_tester_0
  if { [catch {set video_tester_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $video_tester_0 eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
  
  # Create interface connections
  connect_bd_intf_net -intf_net Conn1 [get_bd_intf_pins S_AXI_LITE1] [get_bd_intf_pins axi_vdma_0/S_AXI_LITE]
  connect_bd_intf_net -intf_net Conn2 [get_bd_intf_pins M_AXI_MM2S] [get_bd_intf_pins axi_vdma_0/M_AXI_MM2S]
  connect_bd_intf_net -intf_net axi_vdma_0_M_AXIS_MM2S [get_bd_intf_pins axi_vdma_0/M_AXIS_MM2S] [get_bd_intf_pins axis_data_fifo_0/S_AXIS]
  connect_bd_intf_net -intf_net axis_data_fifo_0_M_AXIS [get_bd_intf_pins axis_data_fifo_0/M_AXIS] [get_bd_intf_pins video_tester_0/m_axis_vid]

  # Create port connections
  connect_bd_net -net axi_resetn1_1 [get_bd_pins axi_resetn1] [get_bd_pins axi_vdma_0/axi_resetn]
  connect_bd_net -net axi_resetn_1 [get_bd_pins axi_resetn] [get_bd_pins axis_data_fifo_0/s_axis_aresetn] [get_bd_pins video_tester_0/aresetn]
  connect_bd_net -net clk_1 [get_bd_pins clk] [get_bd_pins video_tester_0/dvi_clk]
  connect_bd_net -net control_data_1 [get_bd_pins control_data] [get_bd_pins video_tester_0/control_data]
  connect_bd_net -net control_interlace_1 [get_bd_pins control_interlace] [get_bd_pins video_tester_0/control_interlace]
  connect_bd_net -net control_op_1 [get_bd_pins control_op] [get_bd_pins video_tester_0/control_op]
  connect_bd_net -net s_axi_aclk_1 [get_bd_pins s_axi_aclk] [get_bd_pins axi_vdma_0/m_axi_mm2s_aclk] [get_bd_pins axi_vdma_0/m_axis_mm2s_aclk] [get_bd_pins axis_data_fifo_0/s_axis_aclk] [get_bd_pins video_tester_0/m_axis_vid_aclk]
  connect_bd_net -net s_axi_lite_aclk_1 [get_bd_pins s_axi_lite_aclk] [get_bd_pins axi_vdma_0/s_axi_lite_aclk]
  connect_bd_net -net video_tester_0_dbg_state -boundary_type lower [get_bd_pins dbg_state]
  connect_bd_net -net video_tester_0_dvi_active_video [get_bd_pins VGA_DE] [get_bd_pins video_tester_0/dvi_active_video]
  connect_bd_net -net video_tester_0_dvi_hsync [get_bd_pins VGA_HS] [get_bd_pins video_tester_0/dvi_hsync]
  connect_bd_net -net video_tester_0_dvi_rgb [get_bd_pins vid_data] [get_bd_pins video_tester_0/dvi_rgb]
  connect_bd_net -net video_tester_0_dvi_vsync [get_bd_pins VGA_VS] [get_bd_pins video_tester_0/dvi_vsync]

  # Restore current instance
  current_bd_instance $oldCurInst
}


# Procedure to create entire design; Provide argument to make
# procedure reusable. If parentCell is "", will use root.
proc create_root_design { parentCell } {

  variable script_folder
  variable design_name

  if { $parentCell eq "" } {
     set parentCell [get_bd_cells /]
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_msg_id "BD_TCL-100" "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_msg_id "BD_TCL-101" "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj


  # Create interface ports
  set DDR [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:ddrx_rtl:1.0 DDR ]
  set FIXED_IO [ create_bd_intf_port -mode Master -vlnv xilinx.com:display_processing_system7:fixedio_rtl:1.0 FIXED_IO ]

  # Create ports
  set VCAP_B0 [ create_bd_port -dir I VCAP_B0 ]
  set VCAP_B1 [ create_bd_port -dir I VCAP_B1 ]
  set VCAP_B2 [ create_bd_port -dir I VCAP_B2 ]
  set VCAP_B3 [ create_bd_port -dir I VCAP_B3 ]
  set VCAP_B4 [ create_bd_port -dir I VCAP_B4 ]
  set VCAP_B5 [ create_bd_port -dir I VCAP_B5 ]
  set VCAP_B6 [ create_bd_port -dir I VCAP_B6 ]
  set VCAP_B7 [ create_bd_port -dir I VCAP_B7 ]
  set VCAP_G0 [ create_bd_port -dir I VCAP_G0 ]
  set VCAP_G1 [ create_bd_port -dir I VCAP_G1 ]
  set VCAP_G2 [ create_bd_port -dir I VCAP_G2 ]
  set VCAP_G3 [ create_bd_port -dir I VCAP_G3 ]
  set VCAP_G4 [ create_bd_port -dir I VCAP_G4 ]
  set VCAP_G5 [ create_bd_port -dir I VCAP_G5 ]
  set VCAP_G6 [ create_bd_port -dir I VCAP_G6 ]
  set VCAP_G7 [ create_bd_port -dir I VCAP_G7 ]
  set VCAP_HSYNC [ create_bd_port -dir I VCAP_HSYNC ]
  set VCAP_R0 [ create_bd_port -dir I VCAP_R0 ]
  set VCAP_R1 [ create_bd_port -dir I VCAP_R1 ]
  set VCAP_R2 [ create_bd_port -dir I VCAP_R2 ]
  set VCAP_R3 [ create_bd_port -dir I VCAP_R3 ]
  set VCAP_R4 [ create_bd_port -dir I VCAP_R4 ]
  set VCAP_R5 [ create_bd_port -dir I VCAP_R5 ]
  set VCAP_R6 [ create_bd_port -dir I VCAP_R6 ]
  set VCAP_R7 [ create_bd_port -dir I VCAP_R7 ]
  set VCAP_VSYNC [ create_bd_port -dir I VCAP_VSYNC ]
  set VGA_B [ create_bd_port -dir O -from 7 -to 0 VGA_B ]
  set VGA_DE [ create_bd_port -dir O VGA_DE ]
  set VGA_G [ create_bd_port -dir O -from 7 -to 0 VGA_G ]
  set VGA_HS [ create_bd_port -dir O VGA_HS ]
  set VGA_PCLK [ create_bd_port -dir O -type clk VGA_PCLK ]
  set VGA_R [ create_bd_port -dir O -from 7 -to 0 VGA_R ]
  set VGA_VS [ create_bd_port -dir O VGA_VS ]
  set ZORRO_ADDR [ create_bd_port -dir IO -from 22 -to 0 ZORRO_ADDR ]
  set ZORRO_ADDRDIR [ create_bd_port -dir O ZORRO_ADDRDIR ]
  set ZORRO_ADDRDIR2 [ create_bd_port -dir O ZORRO_ADDRDIR2 ]
  set ZORRO_C28D [ create_bd_port -dir I ZORRO_C28D ]
  set ZORRO_DATA [ create_bd_port -dir IO -from 15 -to 0 ZORRO_DATA ]
  set ZORRO_DATADIR [ create_bd_port -dir O ZORRO_DATADIR ]
  set ZORRO_DOE [ create_bd_port -dir I ZORRO_DOE ]
  set ZORRO_E7M [ create_bd_port -dir I ZORRO_E7M ]
  set ZORRO_INT6 [ create_bd_port -dir O ZORRO_INT6 ]
  set ZORRO_NBGN [ create_bd_port -dir I ZORRO_NBGN ]
  set ZORRO_NBRN [ create_bd_port -dir O ZORRO_NBRN ]
  set ZORRO_NCCS [ create_bd_port -dir I ZORRO_NCCS ]
  set ZORRO_NCFGIN [ create_bd_port -dir I ZORRO_NCFGIN ]
  set ZORRO_NCFGOUT [ create_bd_port -dir O ZORRO_NCFGOUT ]
  set ZORRO_NCINH [ create_bd_port -dir O ZORRO_NCINH ]
  set ZORRO_NDS0 [ create_bd_port -dir I ZORRO_NDS0 ]
  set ZORRO_NDS1 [ create_bd_port -dir I ZORRO_NDS1 ]
  set ZORRO_NDTACK [ create_bd_port -dir O ZORRO_NDTACK ]
  set ZORRO_NFCS [ create_bd_port -dir I ZORRO_NFCS ]
  set ZORRO_NIORST [ create_bd_port -dir I ZORRO_NIORST ]
  set ZORRO_NLDS [ create_bd_port -dir I ZORRO_NLDS ]
  set ZORRO_NSLAVE [ create_bd_port -dir O ZORRO_NSLAVE ]
  set ZORRO_NUDS [ create_bd_port -dir I ZORRO_NUDS ]
  set ZORRO_READ [ create_bd_port -dir I ZORRO_READ ]

  # Create instance: MNTZorro_v0_1_S00_AXI_0, and set properties
  set block_name MNTZorro_v0_1_S00_AXI
  set block_cell_name MNTZorro_v0_1_S00_AXI_0
  if { [catch {set MNTZorro_v0_1_S00_AXI_0 [create_bd_cell -type module -reference $block_name $block_cell_name] } errmsg] } {
     catch {common::send_msg_id "BD_TCL-105" "ERROR" "Unable to add referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   } elseif { $MNTZorro_v0_1_S00_AXI_0 eq "" } {
     catch {common::send_msg_id "BD_TCL-106" "ERROR" "Unable to referenced block <$block_name>. Please add the files for ${block_name}'s definition into the project."}
     return 1
   }
  
  # Create instance: axi_protocol_convert_0, and set properties
  set axi_protocol_convert_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_protocol_converter:2.1 axi_protocol_convert_0 ]

  # Create instance: axi_protocol_convert_1, and set properties
  set axi_protocol_convert_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_protocol_converter:2.1 axi_protocol_convert_1 ]
  set_property -dict [ list \
   CONFIG.TRANSLATION_MODE {2} \
 ] $axi_protocol_convert_1

  # Create instance: axi_protocol_convert_2, and set properties
  set axi_protocol_convert_2 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_protocol_converter:2.1 axi_protocol_convert_2 ]

  # Create instance: axi_register_slice_0, and set properties
  set axi_register_slice_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_register_slice:2.1 axi_register_slice_0 ]

  # Create instance: axi_register_slice_1, and set properties
  set axi_register_slice_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_register_slice:2.1 axi_register_slice_1 ]

  # Create instance: axi_register_slice_2, and set properties
  set axi_register_slice_2 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_register_slice:2.1 axi_register_slice_2 ]

  # Create instance: axi_register_slice_3, and set properties
  set axi_register_slice_3 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_register_slice:2.1 axi_register_slice_3 ]

  # Create instance: clk_wiz_0, and set properties
  set clk_wiz_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:clk_wiz:6.0 clk_wiz_0 ]
  set_property -dict [ list \
   CONFIG.CLKOUT1_DRIVES {BUFG} \
   CONFIG.CLKOUT1_JITTER {272.433} \
   CONFIG.CLKOUT1_PHASE_ERROR {261.747} \
   CONFIG.CLKOUT1_REQUESTED_OUT_FREQ {75} \
   CONFIG.CLKOUT2_DRIVES {BUFG} \
   CONFIG.CLKOUT3_DRIVES {BUFG} \
   CONFIG.CLKOUT4_DRIVES {BUFG} \
   CONFIG.CLKOUT5_DRIVES {BUFG} \
   CONFIG.CLKOUT6_DRIVES {BUFG} \
   CONFIG.CLKOUT7_DRIVES {BUFG} \
   CONFIG.MMCM_CLKFBOUT_MULT_F {33} \
   CONFIG.MMCM_CLKOUT0_DIVIDE_F {11} \
   CONFIG.MMCM_COMPENSATION {ZHOLD} \
   CONFIG.MMCM_DIVCLK_DIVIDE {4} \
   CONFIG.PRIMITIVE {PLL} \
   CONFIG.PRIM_SOURCE {Single_ended_clock_capable_pin} \
   CONFIG.SECONDARY_SOURCE {Single_ended_clock_capable_pin} \
   CONFIG.USE_DYN_RECONFIG {true} \
   CONFIG.USE_PHASE_ALIGNMENT {false} \
 ] $clk_wiz_0

  # Create instance: proc_sys_reset_0, and set properties
  set proc_sys_reset_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset:5.0 proc_sys_reset_0 ]

  # Create instance: processing_system7_0, and set properties
  set processing_system7_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7:5.5 processing_system7_0 ]
  set_property -dict [ list \
   CONFIG.PCW_ACT_APU_PERIPHERAL_FREQMHZ {666.666687} \
   CONFIG.PCW_ACT_CAN_PERIPHERAL_FREQMHZ {10.000000} \
   CONFIG.PCW_ACT_DCI_PERIPHERAL_FREQMHZ {10.158730} \
   CONFIG.PCW_ACT_ENET0_PERIPHERAL_FREQMHZ {125.000000} \
   CONFIG.PCW_ACT_ENET1_PERIPHERAL_FREQMHZ {10.000000} \
   CONFIG.PCW_ACT_FPGA0_PERIPHERAL_FREQMHZ {100.000000} \
   CONFIG.PCW_ACT_FPGA1_PERIPHERAL_FREQMHZ {25.000000} \
   CONFIG.PCW_ACT_FPGA2_PERIPHERAL_FREQMHZ {10.000000} \
   CONFIG.PCW_ACT_FPGA3_PERIPHERAL_FREQMHZ {10.000000} \
   CONFIG.PCW_ACT_PCAP_PERIPHERAL_FREQMHZ {200.000000} \
   CONFIG.PCW_ACT_QSPI_PERIPHERAL_FREQMHZ {10.000000} \
   CONFIG.PCW_ACT_SDIO_PERIPHERAL_FREQMHZ {25.000000} \
   CONFIG.PCW_ACT_SMC_PERIPHERAL_FREQMHZ {10.000000} \
   CONFIG.PCW_ACT_SPI_PERIPHERAL_FREQMHZ {10.000000} \
   CONFIG.PCW_ACT_TPIU_PERIPHERAL_FREQMHZ {200.000000} \
   CONFIG.PCW_ACT_TTC0_CLK0_PERIPHERAL_FREQMHZ {111.111115} \
   CONFIG.PCW_ACT_TTC0_CLK1_PERIPHERAL_FREQMHZ {111.111115} \
   CONFIG.PCW_ACT_TTC0_CLK2_PERIPHERAL_FREQMHZ {111.111115} \
   CONFIG.PCW_ACT_TTC1_CLK0_PERIPHERAL_FREQMHZ {111.111115} \
   CONFIG.PCW_ACT_TTC1_CLK1_PERIPHERAL_FREQMHZ {111.111115} \
   CONFIG.PCW_ACT_TTC1_CLK2_PERIPHERAL_FREQMHZ {111.111115} \
   CONFIG.PCW_ACT_UART_PERIPHERAL_FREQMHZ {100.000000} \
   CONFIG.PCW_ACT_WDT_PERIPHERAL_FREQMHZ {111.111115} \
   CONFIG.PCW_APU_CLK_RATIO_ENABLE {6:2:1} \
   CONFIG.PCW_APU_PERIPHERAL_FREQMHZ {666.666} \
   CONFIG.PCW_ARMPLL_CTRL_FBDIV {40} \
   CONFIG.PCW_CAN_PERIPHERAL_DIVISOR0 {1} \
   CONFIG.PCW_CAN_PERIPHERAL_DIVISOR1 {1} \
   CONFIG.PCW_CLK0_FREQ {100000000} \
   CONFIG.PCW_CLK1_FREQ {25000000} \
   CONFIG.PCW_CLK2_FREQ {10000000} \
   CONFIG.PCW_CLK3_FREQ {10000000} \
   CONFIG.PCW_CPU_CPU_6X4X_MAX_RANGE {667} \
   CONFIG.PCW_CPU_CPU_PLL_FREQMHZ {1333.333} \
   CONFIG.PCW_CPU_PERIPHERAL_CLKSRC {ARM PLL} \
   CONFIG.PCW_CPU_PERIPHERAL_DIVISOR0 {2} \
   CONFIG.PCW_DCI_PERIPHERAL_DIVISOR0 {15} \
   CONFIG.PCW_DCI_PERIPHERAL_DIVISOR1 {7} \
   CONFIG.PCW_DDRPLL_CTRL_FBDIV {32} \
   CONFIG.PCW_DDR_DDR_PLL_FREQMHZ {1066.667} \
   CONFIG.PCW_DDR_PERIPHERAL_DIVISOR0 {2} \
   CONFIG.PCW_DDR_RAM_HIGHADDR {0x3FFFFFFF} \
   CONFIG.PCW_ENET0_ENET0_IO {MIO 16 .. 27} \
   CONFIG.PCW_ENET0_GRP_MDIO_ENABLE {1} \
   CONFIG.PCW_ENET0_GRP_MDIO_IO {MIO 52 .. 53} \
   CONFIG.PCW_ENET0_PERIPHERAL_CLKSRC {IO PLL} \
   CONFIG.PCW_ENET0_PERIPHERAL_DIVISOR0 {8} \
   CONFIG.PCW_ENET0_PERIPHERAL_DIVISOR1 {1} \
   CONFIG.PCW_ENET0_PERIPHERAL_ENABLE {1} \
   CONFIG.PCW_ENET0_PERIPHERAL_FREQMHZ {1000 Mbps} \
   CONFIG.PCW_ENET0_RESET_ENABLE {0} \
   CONFIG.PCW_ENET1_GRP_MDIO_ENABLE {0} \
   CONFIG.PCW_ENET1_PERIPHERAL_CLKSRC {IO PLL} \
   CONFIG.PCW_ENET1_PERIPHERAL_DIVISOR0 {1} \
   CONFIG.PCW_ENET1_PERIPHERAL_DIVISOR1 {1} \
   CONFIG.PCW_ENET1_PERIPHERAL_ENABLE {0} \
   CONFIG.PCW_ENET1_PERIPHERAL_FREQMHZ {1000 Mbps} \
   CONFIG.PCW_ENET1_RESET_ENABLE {0} \
   CONFIG.PCW_ENET_RESET_ENABLE {0} \
   CONFIG.PCW_ENET_RESET_SELECT {<Select>} \
   CONFIG.PCW_EN_CLK0_PORT {1} \
   CONFIG.PCW_EN_CLK1_PORT {1} \
   CONFIG.PCW_EN_DDR {1} \
   CONFIG.PCW_EN_EMIO_CD_SDIO0 {0} \
   CONFIG.PCW_EN_EMIO_ENET0 {0} \
   CONFIG.PCW_EN_EMIO_ENET1 {0} \
   CONFIG.PCW_EN_EMIO_GPIO {0} \
   CONFIG.PCW_EN_EMIO_I2C0 {0} \
   CONFIG.PCW_EN_EMIO_TTC0 {0} \
   CONFIG.PCW_EN_EMIO_WDT {0} \
   CONFIG.PCW_EN_ENET0 {1} \
   CONFIG.PCW_EN_ENET1 {0} \
   CONFIG.PCW_EN_GPIO {1} \
   CONFIG.PCW_EN_I2C0 {1} \
   CONFIG.PCW_EN_RST0_PORT {1} \
   CONFIG.PCW_EN_RST1_PORT {0} \
   CONFIG.PCW_EN_SDIO0 {1} \
   CONFIG.PCW_EN_TTC0 {0} \
   CONFIG.PCW_EN_UART1 {1} \
   CONFIG.PCW_EN_WDT {0} \
   CONFIG.PCW_FCLK0_PERIPHERAL_CLKSRC {IO PLL} \
   CONFIG.PCW_FCLK0_PERIPHERAL_DIVISOR0 {5} \
   CONFIG.PCW_FCLK0_PERIPHERAL_DIVISOR1 {2} \
   CONFIG.PCW_FCLK1_PERIPHERAL_DIVISOR0 {8} \
   CONFIG.PCW_FCLK1_PERIPHERAL_DIVISOR1 {5} \
   CONFIG.PCW_FCLK2_PERIPHERAL_DIVISOR0 {1} \
   CONFIG.PCW_FCLK2_PERIPHERAL_DIVISOR1 {1} \
   CONFIG.PCW_FCLK3_PERIPHERAL_DIVISOR0 {1} \
   CONFIG.PCW_FCLK3_PERIPHERAL_DIVISOR1 {1} \
   CONFIG.PCW_FCLK_CLK0_BUF {FALSE} \
   CONFIG.PCW_FCLK_CLK1_BUF {TRUE} \
   CONFIG.PCW_FPGA0_PERIPHERAL_FREQMHZ {100} \
   CONFIG.PCW_FPGA1_PERIPHERAL_FREQMHZ {25} \
   CONFIG.PCW_FPGA_FCLK0_ENABLE {1} \
   CONFIG.PCW_FPGA_FCLK1_ENABLE {1} \
   CONFIG.PCW_FPGA_FCLK2_ENABLE {0} \
   CONFIG.PCW_FPGA_FCLK3_ENABLE {0} \
   CONFIG.PCW_GPIO_EMIO_GPIO_ENABLE {0} \
   CONFIG.PCW_GPIO_EMIO_GPIO_WIDTH {64} \
   CONFIG.PCW_GPIO_MIO_GPIO_ENABLE {1} \
   CONFIG.PCW_GPIO_MIO_GPIO_IO {MIO} \
   CONFIG.PCW_I2C0_GRP_INT_ENABLE {0} \
   CONFIG.PCW_I2C0_GRP_INT_IO {<Select>} \
   CONFIG.PCW_I2C0_I2C0_IO {MIO 50 .. 51} \
   CONFIG.PCW_I2C0_PERIPHERAL_ENABLE {1} \
   CONFIG.PCW_I2C0_RESET_ENABLE {0} \
   CONFIG.PCW_I2C0_RESET_IO {<Select>} \
   CONFIG.PCW_I2C1_RESET_ENABLE {0} \
   CONFIG.PCW_I2C_PERIPHERAL_FREQMHZ {111.111115} \
   CONFIG.PCW_I2C_RESET_ENABLE {0} \
   CONFIG.PCW_I2C_RESET_SELECT {<Select>} \
   CONFIG.PCW_IOPLL_CTRL_FBDIV {30} \
   CONFIG.PCW_IO_IO_PLL_FREQMHZ {1000.000} \
   CONFIG.PCW_MIO_0_DIRECTION {inout} \
   CONFIG.PCW_MIO_0_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_0_PULLUP {enabled} \
   CONFIG.PCW_MIO_0_SLEW {slow} \
   CONFIG.PCW_MIO_10_DIRECTION {inout} \
   CONFIG.PCW_MIO_10_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_10_PULLUP {enabled} \
   CONFIG.PCW_MIO_10_SLEW {slow} \
   CONFIG.PCW_MIO_11_DIRECTION {inout} \
   CONFIG.PCW_MIO_11_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_11_PULLUP {enabled} \
   CONFIG.PCW_MIO_11_SLEW {slow} \
   CONFIG.PCW_MIO_12_DIRECTION {inout} \
   CONFIG.PCW_MIO_12_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_12_PULLUP {enabled} \
   CONFIG.PCW_MIO_12_SLEW {slow} \
   CONFIG.PCW_MIO_13_DIRECTION {inout} \
   CONFIG.PCW_MIO_13_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_13_PULLUP {enabled} \
   CONFIG.PCW_MIO_13_SLEW {slow} \
   CONFIG.PCW_MIO_14_DIRECTION {inout} \
   CONFIG.PCW_MIO_14_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_14_PULLUP {enabled} \
   CONFIG.PCW_MIO_14_SLEW {slow} \
   CONFIG.PCW_MIO_15_DIRECTION {inout} \
   CONFIG.PCW_MIO_15_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_15_PULLUP {enabled} \
   CONFIG.PCW_MIO_15_SLEW {slow} \
   CONFIG.PCW_MIO_16_DIRECTION {out} \
   CONFIG.PCW_MIO_16_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_16_PULLUP {enabled} \
   CONFIG.PCW_MIO_16_SLEW {slow} \
   CONFIG.PCW_MIO_17_DIRECTION {out} \
   CONFIG.PCW_MIO_17_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_17_PULLUP {enabled} \
   CONFIG.PCW_MIO_17_SLEW {slow} \
   CONFIG.PCW_MIO_18_DIRECTION {out} \
   CONFIG.PCW_MIO_18_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_18_PULLUP {enabled} \
   CONFIG.PCW_MIO_18_SLEW {slow} \
   CONFIG.PCW_MIO_19_DIRECTION {out} \
   CONFIG.PCW_MIO_19_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_19_PULLUP {enabled} \
   CONFIG.PCW_MIO_19_SLEW {slow} \
   CONFIG.PCW_MIO_1_DIRECTION {inout} \
   CONFIG.PCW_MIO_1_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_1_PULLUP {enabled} \
   CONFIG.PCW_MIO_1_SLEW {slow} \
   CONFIG.PCW_MIO_20_DIRECTION {out} \
   CONFIG.PCW_MIO_20_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_20_PULLUP {enabled} \
   CONFIG.PCW_MIO_20_SLEW {slow} \
   CONFIG.PCW_MIO_21_DIRECTION {out} \
   CONFIG.PCW_MIO_21_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_21_PULLUP {enabled} \
   CONFIG.PCW_MIO_21_SLEW {slow} \
   CONFIG.PCW_MIO_22_DIRECTION {in} \
   CONFIG.PCW_MIO_22_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_22_PULLUP {enabled} \
   CONFIG.PCW_MIO_22_SLEW {slow} \
   CONFIG.PCW_MIO_23_DIRECTION {in} \
   CONFIG.PCW_MIO_23_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_23_PULLUP {enabled} \
   CONFIG.PCW_MIO_23_SLEW {slow} \
   CONFIG.PCW_MIO_24_DIRECTION {in} \
   CONFIG.PCW_MIO_24_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_24_PULLUP {enabled} \
   CONFIG.PCW_MIO_24_SLEW {slow} \
   CONFIG.PCW_MIO_25_DIRECTION {in} \
   CONFIG.PCW_MIO_25_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_25_PULLUP {enabled} \
   CONFIG.PCW_MIO_25_SLEW {slow} \
   CONFIG.PCW_MIO_26_DIRECTION {in} \
   CONFIG.PCW_MIO_26_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_26_PULLUP {enabled} \
   CONFIG.PCW_MIO_26_SLEW {slow} \
   CONFIG.PCW_MIO_27_DIRECTION {in} \
   CONFIG.PCW_MIO_27_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_27_PULLUP {enabled} \
   CONFIG.PCW_MIO_27_SLEW {slow} \
   CONFIG.PCW_MIO_28_DIRECTION {inout} \
   CONFIG.PCW_MIO_28_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_28_PULLUP {enabled} \
   CONFIG.PCW_MIO_28_SLEW {slow} \
   CONFIG.PCW_MIO_29_DIRECTION {inout} \
   CONFIG.PCW_MIO_29_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_29_PULLUP {enabled} \
   CONFIG.PCW_MIO_29_SLEW {slow} \
   CONFIG.PCW_MIO_2_DIRECTION {inout} \
   CONFIG.PCW_MIO_2_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_2_PULLUP {disabled} \
   CONFIG.PCW_MIO_2_SLEW {slow} \
   CONFIG.PCW_MIO_30_DIRECTION {inout} \
   CONFIG.PCW_MIO_30_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_30_PULLUP {enabled} \
   CONFIG.PCW_MIO_30_SLEW {slow} \
   CONFIG.PCW_MIO_31_DIRECTION {inout} \
   CONFIG.PCW_MIO_31_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_31_PULLUP {enabled} \
   CONFIG.PCW_MIO_31_SLEW {slow} \
   CONFIG.PCW_MIO_32_DIRECTION {inout} \
   CONFIG.PCW_MIO_32_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_32_PULLUP {enabled} \
   CONFIG.PCW_MIO_32_SLEW {slow} \
   CONFIG.PCW_MIO_33_DIRECTION {inout} \
   CONFIG.PCW_MIO_33_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_33_PULLUP {enabled} \
   CONFIG.PCW_MIO_33_SLEW {slow} \
   CONFIG.PCW_MIO_34_DIRECTION {inout} \
   CONFIG.PCW_MIO_34_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_34_PULLUP {enabled} \
   CONFIG.PCW_MIO_34_SLEW {slow} \
   CONFIG.PCW_MIO_35_DIRECTION {inout} \
   CONFIG.PCW_MIO_35_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_35_PULLUP {enabled} \
   CONFIG.PCW_MIO_35_SLEW {slow} \
   CONFIG.PCW_MIO_36_DIRECTION {inout} \
   CONFIG.PCW_MIO_36_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_36_PULLUP {enabled} \
   CONFIG.PCW_MIO_36_SLEW {slow} \
   CONFIG.PCW_MIO_37_DIRECTION {inout} \
   CONFIG.PCW_MIO_37_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_37_PULLUP {enabled} \
   CONFIG.PCW_MIO_37_SLEW {slow} \
   CONFIG.PCW_MIO_38_DIRECTION {inout} \
   CONFIG.PCW_MIO_38_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_38_PULLUP {enabled} \
   CONFIG.PCW_MIO_38_SLEW {slow} \
   CONFIG.PCW_MIO_39_DIRECTION {inout} \
   CONFIG.PCW_MIO_39_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_39_PULLUP {enabled} \
   CONFIG.PCW_MIO_39_SLEW {slow} \
   CONFIG.PCW_MIO_3_DIRECTION {inout} \
   CONFIG.PCW_MIO_3_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_3_PULLUP {disabled} \
   CONFIG.PCW_MIO_3_SLEW {slow} \
   CONFIG.PCW_MIO_40_DIRECTION {inout} \
   CONFIG.PCW_MIO_40_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_40_PULLUP {enabled} \
   CONFIG.PCW_MIO_40_SLEW {slow} \
   CONFIG.PCW_MIO_41_DIRECTION {inout} \
   CONFIG.PCW_MIO_41_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_41_PULLUP {enabled} \
   CONFIG.PCW_MIO_41_SLEW {slow} \
   CONFIG.PCW_MIO_42_DIRECTION {inout} \
   CONFIG.PCW_MIO_42_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_42_PULLUP {enabled} \
   CONFIG.PCW_MIO_42_SLEW {slow} \
   CONFIG.PCW_MIO_43_DIRECTION {inout} \
   CONFIG.PCW_MIO_43_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_43_PULLUP {enabled} \
   CONFIG.PCW_MIO_43_SLEW {slow} \
   CONFIG.PCW_MIO_44_DIRECTION {inout} \
   CONFIG.PCW_MIO_44_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_44_PULLUP {enabled} \
   CONFIG.PCW_MIO_44_SLEW {slow} \
   CONFIG.PCW_MIO_45_DIRECTION {inout} \
   CONFIG.PCW_MIO_45_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_45_PULLUP {enabled} \
   CONFIG.PCW_MIO_45_SLEW {slow} \
   CONFIG.PCW_MIO_46_DIRECTION {inout} \
   CONFIG.PCW_MIO_46_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_46_PULLUP {enabled} \
   CONFIG.PCW_MIO_46_SLEW {slow} \
   CONFIG.PCW_MIO_47_DIRECTION {inout} \
   CONFIG.PCW_MIO_47_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_47_PULLUP {enabled} \
   CONFIG.PCW_MIO_47_SLEW {slow} \
   CONFIG.PCW_MIO_48_DIRECTION {out} \
   CONFIG.PCW_MIO_48_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_48_PULLUP {enabled} \
   CONFIG.PCW_MIO_48_SLEW {slow} \
   CONFIG.PCW_MIO_49_DIRECTION {in} \
   CONFIG.PCW_MIO_49_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_49_PULLUP {enabled} \
   CONFIG.PCW_MIO_49_SLEW {slow} \
   CONFIG.PCW_MIO_4_DIRECTION {inout} \
   CONFIG.PCW_MIO_4_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_4_PULLUP {disabled} \
   CONFIG.PCW_MIO_4_SLEW {slow} \
   CONFIG.PCW_MIO_50_DIRECTION {inout} \
   CONFIG.PCW_MIO_50_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_50_PULLUP {enabled} \
   CONFIG.PCW_MIO_50_SLEW {slow} \
   CONFIG.PCW_MIO_51_DIRECTION {inout} \
   CONFIG.PCW_MIO_51_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_51_PULLUP {enabled} \
   CONFIG.PCW_MIO_51_SLEW {slow} \
   CONFIG.PCW_MIO_52_DIRECTION {out} \
   CONFIG.PCW_MIO_52_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_52_PULLUP {enabled} \
   CONFIG.PCW_MIO_52_SLEW {slow} \
   CONFIG.PCW_MIO_53_DIRECTION {inout} \
   CONFIG.PCW_MIO_53_IOTYPE {LVCMOS 1.8V} \
   CONFIG.PCW_MIO_53_PULLUP {enabled} \
   CONFIG.PCW_MIO_53_SLEW {slow} \
   CONFIG.PCW_MIO_5_DIRECTION {inout} \
   CONFIG.PCW_MIO_5_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_5_PULLUP {disabled} \
   CONFIG.PCW_MIO_5_SLEW {slow} \
   CONFIG.PCW_MIO_6_DIRECTION {inout} \
   CONFIG.PCW_MIO_6_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_6_PULLUP {disabled} \
   CONFIG.PCW_MIO_6_SLEW {slow} \
   CONFIG.PCW_MIO_7_DIRECTION {out} \
   CONFIG.PCW_MIO_7_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_7_PULLUP {disabled} \
   CONFIG.PCW_MIO_7_SLEW {slow} \
   CONFIG.PCW_MIO_8_DIRECTION {out} \
   CONFIG.PCW_MIO_8_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_8_PULLUP {disabled} \
   CONFIG.PCW_MIO_8_SLEW {slow} \
   CONFIG.PCW_MIO_9_DIRECTION {inout} \
   CONFIG.PCW_MIO_9_IOTYPE {LVCMOS 3.3V} \
   CONFIG.PCW_MIO_9_PULLUP {enabled} \
   CONFIG.PCW_MIO_9_SLEW {slow} \
   CONFIG.PCW_MIO_TREE_PERIPHERALS {GPIO#GPIO#GPIO#GPIO#GPIO#GPIO#GPIO#GPIO#GPIO#GPIO#GPIO#GPIO#GPIO#GPIO#GPIO#GPIO#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#Enet 0#GPIO#GPIO#GPIO#GPIO#GPIO#GPIO#GPIO#GPIO#GPIO#GPIO#GPIO#GPIO#SD 0#SD 0#SD 0#SD 0#SD 0#SD 0#GPIO#GPIO#UART 1#UART 1#I2C 0#I2C 0#Enet 0#Enet 0} \
   CONFIG.PCW_MIO_TREE_SIGNALS {gpio[0]#gpio[1]#gpio[2]#gpio[3]#gpio[4]#gpio[5]#gpio[6]#gpio[7]#gpio[8]#gpio[9]#gpio[10]#gpio[11]#gpio[12]#gpio[13]#gpio[14]#gpio[15]#tx_clk#txd[0]#txd[1]#txd[2]#txd[3]#tx_ctl#rx_clk#rxd[0]#rxd[1]#rxd[2]#rxd[3]#rx_ctl#gpio[28]#gpio[29]#gpio[30]#gpio[31]#gpio[32]#gpio[33]#gpio[34]#gpio[35]#gpio[36]#gpio[37]#gpio[38]#gpio[39]#clk#cmd#data[0]#data[1]#data[2]#data[3]#gpio[46]#gpio[47]#tx#rx#scl#sda#mdc#mdio} \
   CONFIG.PCW_P2F_ENET0_INTR {1} \
   CONFIG.PCW_PCAP_PERIPHERAL_DIVISOR0 {5} \
   CONFIG.PCW_PRESET_BANK1_VOLTAGE {LVCMOS 1.8V} \
   CONFIG.PCW_QSPI_PERIPHERAL_DIVISOR0 {1} \
   CONFIG.PCW_SD0_GRP_CD_ENABLE {0} \
   CONFIG.PCW_SD0_GRP_POW_ENABLE {0} \
   CONFIG.PCW_SD0_GRP_WP_ENABLE {0} \
   CONFIG.PCW_SD0_PERIPHERAL_ENABLE {1} \
   CONFIG.PCW_SD0_SD0_IO {MIO 40 .. 45} \
   CONFIG.PCW_SDIO_PERIPHERAL_CLKSRC {IO PLL} \
   CONFIG.PCW_SDIO_PERIPHERAL_DIVISOR0 {40} \
   CONFIG.PCW_SDIO_PERIPHERAL_FREQMHZ {25} \
   CONFIG.PCW_SDIO_PERIPHERAL_VALID {1} \
   CONFIG.PCW_SMC_PERIPHERAL_DIVISOR0 {1} \
   CONFIG.PCW_SPI_PERIPHERAL_DIVISOR0 {1} \
   CONFIG.PCW_S_AXI_HP0_DATA_WIDTH {32} \
   CONFIG.PCW_S_AXI_HP1_DATA_WIDTH {32} \
   CONFIG.PCW_TPIU_PERIPHERAL_DIVISOR0 {1} \
   CONFIG.PCW_TTC0_CLK0_PERIPHERAL_FREQMHZ {133.333333} \
   CONFIG.PCW_TTC0_CLK1_PERIPHERAL_FREQMHZ {133.333333} \
   CONFIG.PCW_TTC0_CLK2_PERIPHERAL_FREQMHZ {133.333333} \
   CONFIG.PCW_TTC0_PERIPHERAL_ENABLE {0} \
   CONFIG.PCW_TTC0_TTC0_IO {<Select>} \
   CONFIG.PCW_TTC_PERIPHERAL_FREQMHZ {50} \
   CONFIG.PCW_UART1_GRP_FULL_ENABLE {0} \
   CONFIG.PCW_UART1_PERIPHERAL_ENABLE {1} \
   CONFIG.PCW_UART1_UART1_IO {MIO 48 .. 49} \
   CONFIG.PCW_UART_PERIPHERAL_DIVISOR0 {10} \
   CONFIG.PCW_UART_PERIPHERAL_FREQMHZ {100} \
   CONFIG.PCW_UART_PERIPHERAL_VALID {1} \
   CONFIG.PCW_UIPARAM_ACT_DDR_FREQ_MHZ {533.333374} \
   CONFIG.PCW_UIPARAM_DDR_AL {0} \
   CONFIG.PCW_UIPARAM_DDR_BANK_ADDR_COUNT {3} \
   CONFIG.PCW_UIPARAM_DDR_BL {8} \
   CONFIG.PCW_UIPARAM_DDR_BOARD_DELAY0 {0.271} \
   CONFIG.PCW_UIPARAM_DDR_BOARD_DELAY1 {0.259} \
   CONFIG.PCW_UIPARAM_DDR_BOARD_DELAY2 {0.219} \
   CONFIG.PCW_UIPARAM_DDR_BOARD_DELAY3 {0.207} \
   CONFIG.PCW_UIPARAM_DDR_BUS_WIDTH {32 Bit} \
   CONFIG.PCW_UIPARAM_DDR_CL {7} \
   CONFIG.PCW_UIPARAM_DDR_CLOCK_STOP_EN {0} \
   CONFIG.PCW_UIPARAM_DDR_COL_ADDR_COUNT {10} \
   CONFIG.PCW_UIPARAM_DDR_CWL {6} \
   CONFIG.PCW_UIPARAM_DDR_DEVICE_CAPACITY {4096 MBits} \
   CONFIG.PCW_UIPARAM_DDR_DQS_TO_CLK_DELAY_0 {0.229} \
   CONFIG.PCW_UIPARAM_DDR_DQS_TO_CLK_DELAY_1 {0.250} \
   CONFIG.PCW_UIPARAM_DDR_DQS_TO_CLK_DELAY_2 {0.121} \
   CONFIG.PCW_UIPARAM_DDR_DQS_TO_CLK_DELAY_3 {0.146} \
   CONFIG.PCW_UIPARAM_DDR_DRAM_WIDTH {16 Bits} \
   CONFIG.PCW_UIPARAM_DDR_ECC {Disabled} \
   CONFIG.PCW_UIPARAM_DDR_ENABLE {1} \
   CONFIG.PCW_UIPARAM_DDR_FREQ_MHZ {533.333} \
   CONFIG.PCW_UIPARAM_DDR_HIGH_TEMP {Normal (0-85)} \
   CONFIG.PCW_UIPARAM_DDR_MEMORY_TYPE {DDR 3} \
   CONFIG.PCW_UIPARAM_DDR_PARTNO {MT41J256M16 RE-125} \
   CONFIG.PCW_UIPARAM_DDR_ROW_ADDR_COUNT {15} \
   CONFIG.PCW_UIPARAM_DDR_SPEED_BIN {DDR3_1066F} \
   CONFIG.PCW_UIPARAM_DDR_TRAIN_DATA_EYE {1} \
   CONFIG.PCW_UIPARAM_DDR_TRAIN_READ_GATE {1} \
   CONFIG.PCW_UIPARAM_DDR_TRAIN_WRITE_LEVEL {1} \
   CONFIG.PCW_UIPARAM_DDR_T_FAW {40.0} \
   CONFIG.PCW_UIPARAM_DDR_T_RAS_MIN {35.0} \
   CONFIG.PCW_UIPARAM_DDR_T_RC {48.91} \
   CONFIG.PCW_UIPARAM_DDR_T_RCD {7} \
   CONFIG.PCW_UIPARAM_DDR_T_RP {7} \
   CONFIG.PCW_UIPARAM_DDR_USE_INTERNAL_VREF {0} \
   CONFIG.PCW_USB0_RESET_ENABLE {0} \
   CONFIG.PCW_USB1_RESET_ENABLE {0} \
   CONFIG.PCW_USB_RESET_ENABLE {0} \
   CONFIG.PCW_USE_AXI_NONSECURE {0} \
   CONFIG.PCW_USE_FABRIC_INTERRUPT {0} \
   CONFIG.PCW_USE_M_AXI_GP0 {1} \
   CONFIG.PCW_USE_M_AXI_GP1 {1} \
   CONFIG.PCW_USE_S_AXI_GP0 {0} \
   CONFIG.PCW_USE_S_AXI_HP0 {1} \
   CONFIG.PCW_USE_S_AXI_HP1 {1} \
   CONFIG.PCW_WDT_PERIPHERAL_ENABLE {0} \
   CONFIG.PCW_WDT_PERIPHERAL_FREQMHZ {133.333333} \
   CONFIG.PCW_WDT_WDT_IO {<Select>} \
 ] $processing_system7_0

  # Create instance: ps7_0_axi_periph, and set properties
  set ps7_0_axi_periph [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 ps7_0_axi_periph ]
  set_property -dict [ list \
   CONFIG.M00_HAS_DATA_FIFO {0} \
   CONFIG.M00_HAS_REGSLICE {3} \
   CONFIG.M01_HAS_DATA_FIFO {0} \
   CONFIG.M01_HAS_REGSLICE {3} \
   CONFIG.NUM_MI {2} \
   CONFIG.S00_HAS_DATA_FIFO {1} \
   CONFIG.S00_HAS_REGSLICE {4} \
 ] $ps7_0_axi_periph

  # Create instance: rst_ps7_0_25M, and set properties
  set rst_ps7_0_25M [ create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset:5.0 rst_ps7_0_25M ]
  set_property -dict [ list \
   CONFIG.C_NUM_INTERCONNECT_ARESETN {1} \
   CONFIG.C_NUM_PERP_ARESETN {1} \
 ] $rst_ps7_0_25M

  # Create instance: video_subsystem
  create_hier_cell_video_subsystem [current_bd_instance .] video_subsystem

  # Create instance: xlslice_0, and set properties
  set xlslice_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice:1.0 xlslice_0 ]
  set_property -dict [ list \
   CONFIG.DIN_FROM {23} \
   CONFIG.DIN_TO {16} \
   CONFIG.DIN_WIDTH {32} \
   CONFIG.DOUT_WIDTH {8} \
 ] $xlslice_0

  # Create instance: xlslice_1, and set properties
  set xlslice_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice:1.0 xlslice_1 ]
  set_property -dict [ list \
   CONFIG.DIN_FROM {15} \
   CONFIG.DIN_TO {8} \
   CONFIG.DIN_WIDTH {32} \
   CONFIG.DOUT_WIDTH {8} \
 ] $xlslice_1

  # Create instance: xlslice_2, and set properties
  set xlslice_2 [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice:1.0 xlslice_2 ]
  set_property -dict [ list \
   CONFIG.DIN_FROM {7} \
   CONFIG.DIN_TO {0} \
   CONFIG.DIN_WIDTH {32} \
   CONFIG.DOUT_WIDTH {8} \
 ] $xlslice_2

  # Create interface connections
  connect_bd_intf_net -intf_net MNTZorro_v0_1_S00_AXI_0_m00_axi [get_bd_intf_pins MNTZorro_v0_1_S00_AXI_0/m00_axi] [get_bd_intf_pins axi_protocol_convert_2/S_AXI]
  connect_bd_intf_net -intf_net axi_protocol_convert_0_M_AXI [get_bd_intf_pins MNTZorro_v0_1_S00_AXI_0/S_AXI] [get_bd_intf_pins axi_protocol_convert_0/M_AXI]
  connect_bd_intf_net -intf_net axi_protocol_convert_1_M_AXI [get_bd_intf_pins axi_protocol_convert_1/M_AXI] [get_bd_intf_pins axi_register_slice_1/S_AXI]
  connect_bd_intf_net -intf_net axi_protocol_convert_2_M_AXI [get_bd_intf_pins axi_protocol_convert_2/M_AXI] [get_bd_intf_pins axi_register_slice_3/S_AXI]
  connect_bd_intf_net -intf_net axi_register_slice_0_M_AXI [get_bd_intf_pins axi_register_slice_0/M_AXI] [get_bd_intf_pins axi_register_slice_2/S_AXI]
  connect_bd_intf_net -intf_net axi_register_slice_1_M_AXI [get_bd_intf_pins axi_register_slice_1/M_AXI] [get_bd_intf_pins processing_system7_0/S_AXI_HP0]
  connect_bd_intf_net -intf_net axi_register_slice_2_M_AXI [get_bd_intf_pins axi_protocol_convert_1/S_AXI] [get_bd_intf_pins axi_register_slice_2/M_AXI]
  connect_bd_intf_net -intf_net axi_register_slice_3_M_AXI [get_bd_intf_pins axi_register_slice_3/M_AXI] [get_bd_intf_pins processing_system7_0/S_AXI_HP1]
  connect_bd_intf_net -intf_net processing_system7_0_DDR [get_bd_intf_ports DDR] [get_bd_intf_pins processing_system7_0/DDR]
  connect_bd_intf_net -intf_net processing_system7_0_FIXED_IO [get_bd_intf_ports FIXED_IO] [get_bd_intf_pins processing_system7_0/FIXED_IO]
  connect_bd_intf_net -intf_net processing_system7_0_M_AXI_GP0 [get_bd_intf_pins axi_protocol_convert_0/S_AXI] [get_bd_intf_pins processing_system7_0/M_AXI_GP0]
  connect_bd_intf_net -intf_net processing_system7_0_M_AXI_GP1 [get_bd_intf_pins processing_system7_0/M_AXI_GP1] [get_bd_intf_pins ps7_0_axi_periph/S00_AXI]
  connect_bd_intf_net -intf_net ps7_0_axi_periph_M00_AXI [get_bd_intf_pins clk_wiz_0/s_axi_lite] [get_bd_intf_pins ps7_0_axi_periph/M00_AXI]
  connect_bd_intf_net -intf_net ps7_0_axi_periph_M01_AXI [get_bd_intf_pins ps7_0_axi_periph/M01_AXI] [get_bd_intf_pins video_subsystem/S_AXI_LITE1]
  connect_bd_intf_net -intf_net video_subsystem_M_AXI_MM2S [get_bd_intf_pins axi_register_slice_0/S_AXI] [get_bd_intf_pins video_subsystem/M_AXI_MM2S]

  # Create port connections
  connect_bd_net -net M00_ARESETN_1 [get_bd_pins ps7_0_axi_periph/ARESETN] [get_bd_pins ps7_0_axi_periph/M00_ARESETN] [get_bd_pins ps7_0_axi_periph/M01_ARESETN] [get_bd_pins ps7_0_axi_periph/S00_ARESETN] [get_bd_pins rst_ps7_0_25M/interconnect_aresetn]
  connect_bd_net -net MNTZorro_v0_1_S00_AXI_0_ZORRO_ADDRDIR [get_bd_ports ZORRO_ADDRDIR] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_ADDRDIR]
  connect_bd_net -net MNTZorro_v0_1_S00_AXI_0_ZORRO_ADDRDIR2 [get_bd_ports ZORRO_ADDRDIR2] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_ADDRDIR2]
  connect_bd_net -net MNTZorro_v0_1_S00_AXI_0_ZORRO_DATADIR [get_bd_ports ZORRO_DATADIR] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_DATADIR]
  connect_bd_net -net MNTZorro_v0_1_S00_AXI_0_ZORRO_INT6 [get_bd_ports ZORRO_INT6] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_INT6]
  connect_bd_net -net MNTZorro_v0_1_S00_AXI_0_ZORRO_NBRN [get_bd_ports ZORRO_NBRN] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_NBRN]
  connect_bd_net -net MNTZorro_v0_1_S00_AXI_0_ZORRO_NCFGOUT [get_bd_ports ZORRO_NCFGOUT] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_NCFGOUT]
  connect_bd_net -net MNTZorro_v0_1_S00_AXI_0_ZORRO_NCINH [get_bd_ports ZORRO_NCINH] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_NCINH]
  connect_bd_net -net MNTZorro_v0_1_S00_AXI_0_ZORRO_NDTACK [get_bd_ports ZORRO_NDTACK] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_NDTACK]
  connect_bd_net -net MNTZorro_v0_1_S00_AXI_0_ZORRO_NSLAVE [get_bd_ports ZORRO_NSLAVE] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_NSLAVE]
  connect_bd_net -net MNTZorro_v0_1_S00_AXI_0_video_control_data [get_bd_pins MNTZorro_v0_1_S00_AXI_0/video_control_data] [get_bd_pins video_subsystem/control_data]
  connect_bd_net -net MNTZorro_v0_1_S00_AXI_0_video_control_interlace [get_bd_pins MNTZorro_v0_1_S00_AXI_0/video_control_interlace] [get_bd_pins video_subsystem/control_interlace]
  connect_bd_net -net MNTZorro_v0_1_S00_AXI_0_video_control_op [get_bd_pins MNTZorro_v0_1_S00_AXI_0/video_control_op] [get_bd_pins video_subsystem/control_op]
  connect_bd_net -net Net [get_bd_ports ZORRO_ADDR] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_ADDR]
  connect_bd_net -net Net1 [get_bd_ports ZORRO_DATA] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_DATA]
  connect_bd_net -net VCAP_B0_0_1 [get_bd_ports VCAP_B0] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_B0]
  connect_bd_net -net VCAP_B1_0_1 [get_bd_ports VCAP_B1] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_B1]
  connect_bd_net -net VCAP_B2_0_1 [get_bd_ports VCAP_B2] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_B2]
  connect_bd_net -net VCAP_B3_0_1 [get_bd_ports VCAP_B3] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_B3]
  connect_bd_net -net VCAP_B4_0_1 [get_bd_ports VCAP_B4] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_B4]
  connect_bd_net -net VCAP_B5_0_1 [get_bd_ports VCAP_B5] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_B5]
  connect_bd_net -net VCAP_B6_0_1 [get_bd_ports VCAP_B6] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_B6]
  connect_bd_net -net VCAP_B7_0_1 [get_bd_ports VCAP_B7] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_B7]
  connect_bd_net -net VCAP_G0_0_1 [get_bd_ports VCAP_G0] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_G0]
  connect_bd_net -net VCAP_G1_0_1 [get_bd_ports VCAP_G1] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_G1]
  connect_bd_net -net VCAP_G2_0_1 [get_bd_ports VCAP_G2] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_G2]
  connect_bd_net -net VCAP_G3_0_1 [get_bd_ports VCAP_G3] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_G3]
  connect_bd_net -net VCAP_G4_0_1 [get_bd_ports VCAP_G4] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_G4]
  connect_bd_net -net VCAP_G5_0_1 [get_bd_ports VCAP_G5] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_G5]
  connect_bd_net -net VCAP_G6_0_1 [get_bd_ports VCAP_G6] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_G6]
  connect_bd_net -net VCAP_G7_0_1 [get_bd_ports VCAP_G7] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_G7]
  connect_bd_net -net VCAP_HSYNC_0_1 [get_bd_ports VCAP_HSYNC] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_HSYNC]
  connect_bd_net -net VCAP_R0_0_1 [get_bd_ports VCAP_R0] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_R0]
  connect_bd_net -net VCAP_R1_0_1 [get_bd_ports VCAP_R1] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_R1]
  connect_bd_net -net VCAP_R2_0_1 [get_bd_ports VCAP_R2] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_R2]
  connect_bd_net -net VCAP_R3_0_1 [get_bd_ports VCAP_R3] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_R3]
  connect_bd_net -net VCAP_R4_0_1 [get_bd_ports VCAP_R4] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_R4]
  connect_bd_net -net VCAP_R5_0_1 [get_bd_ports VCAP_R5] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_R5]
  connect_bd_net -net VCAP_R6_0_1 [get_bd_ports VCAP_R6] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_R6]
  connect_bd_net -net VCAP_R7_0_1 [get_bd_ports VCAP_R7] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_R7]
  connect_bd_net -net VCAP_VSYNC_0_1 [get_bd_ports VCAP_VSYNC] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/VCAP_VSYNC]
  connect_bd_net -net ZORRO_C28D_0_1 [get_bd_ports ZORRO_C28D] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_C28D]
  connect_bd_net -net ZORRO_DOE_1 [get_bd_ports ZORRO_DOE] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_DOE]
  connect_bd_net -net ZORRO_E7M_1 [get_bd_ports ZORRO_E7M] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_E7M]
  connect_bd_net -net ZORRO_NBGN_0_1 [get_bd_ports ZORRO_NBGN] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_NBGN]
  connect_bd_net -net ZORRO_NCCS_1 [get_bd_ports ZORRO_NCCS] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_NCCS]
  connect_bd_net -net ZORRO_NCFGIN_1 [get_bd_ports ZORRO_NCFGIN] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_NCFGIN]
  connect_bd_net -net ZORRO_NDS0_1 [get_bd_ports ZORRO_NDS0] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_NDS0]
  connect_bd_net -net ZORRO_NDS1_1 [get_bd_ports ZORRO_NDS1] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_NDS1]
  connect_bd_net -net ZORRO_NFCS_1 [get_bd_ports ZORRO_NFCS] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_NFCS]
  connect_bd_net -net ZORRO_NIORST_1 [get_bd_ports ZORRO_NIORST] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_NIORST]
  connect_bd_net -net ZORRO_NLDS_1 [get_bd_ports ZORRO_NLDS] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_NLDS]
  connect_bd_net -net ZORRO_NUDS_1 [get_bd_ports ZORRO_NUDS] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_NUDS]
  connect_bd_net -net ZORRO_READ_1 [get_bd_ports ZORRO_READ] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/ZORRO_READ]
  connect_bd_net -net clk_1 [get_bd_ports VGA_PCLK] [get_bd_pins clk_wiz_0/clk_out1] [get_bd_pins video_subsystem/clk]
  connect_bd_net -net proc_sys_reset_1_peripheral_aresetn [get_bd_pins MNTZorro_v0_1_S00_AXI_0/S_AXI_ARESETN] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/m00_axi_aresetn] [get_bd_pins axi_protocol_convert_0/aresetn] [get_bd_pins axi_protocol_convert_1/aresetn] [get_bd_pins axi_protocol_convert_2/aresetn] [get_bd_pins axi_register_slice_0/aresetn] [get_bd_pins axi_register_slice_1/aresetn] [get_bd_pins axi_register_slice_2/aresetn] [get_bd_pins axi_register_slice_3/aresetn] [get_bd_pins proc_sys_reset_0/peripheral_aresetn] [get_bd_pins video_subsystem/axi_resetn]
  connect_bd_net -net processing_system7_0_FCLK_CLK0 [get_bd_pins MNTZorro_v0_1_S00_AXI_0/S_AXI_ACLK] [get_bd_pins MNTZorro_v0_1_S00_AXI_0/m00_axi_aclk] [get_bd_pins axi_protocol_convert_0/aclk] [get_bd_pins axi_protocol_convert_1/aclk] [get_bd_pins axi_protocol_convert_2/aclk] [get_bd_pins axi_register_slice_0/aclk] [get_bd_pins axi_register_slice_1/aclk] [get_bd_pins axi_register_slice_2/aclk] [get_bd_pins axi_register_slice_3/aclk] [get_bd_pins clk_wiz_0/clk_in1] [get_bd_pins proc_sys_reset_0/slowest_sync_clk] [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins processing_system7_0/M_AXI_GP0_ACLK] [get_bd_pins processing_system7_0/S_AXI_HP0_ACLK] [get_bd_pins processing_system7_0/S_AXI_HP1_ACLK] [get_bd_pins video_subsystem/s_axi_aclk]
  connect_bd_net -net processing_system7_0_FCLK_CLK1 [get_bd_pins clk_wiz_0/s_axi_aclk] [get_bd_pins processing_system7_0/FCLK_CLK1] [get_bd_pins processing_system7_0/M_AXI_GP1_ACLK] [get_bd_pins ps7_0_axi_periph/ACLK] [get_bd_pins ps7_0_axi_periph/M00_ACLK] [get_bd_pins ps7_0_axi_periph/M01_ACLK] [get_bd_pins ps7_0_axi_periph/S00_ACLK] [get_bd_pins rst_ps7_0_25M/slowest_sync_clk] [get_bd_pins video_subsystem/s_axi_lite_aclk]
  connect_bd_net -net processing_system7_0_FCLK_RESET0_N [get_bd_pins proc_sys_reset_0/ext_reset_in] [get_bd_pins processing_system7_0/FCLK_RESET0_N] [get_bd_pins rst_ps7_0_25M/ext_reset_in]
  connect_bd_net -net rst_ps7_0_25M_peripheral_aresetn [get_bd_pins clk_wiz_0/s_axi_aresetn] [get_bd_pins rst_ps7_0_25M/peripheral_aresetn] [get_bd_pins video_subsystem/axi_resetn1]
  connect_bd_net -net v_axi4s_vid_out_0_vid_data [get_bd_pins video_subsystem/vid_data] [get_bd_pins xlslice_0/Din] [get_bd_pins xlslice_1/Din] [get_bd_pins xlslice_2/Din]
  connect_bd_net -net video_subsystem_VGA_DE [get_bd_ports VGA_DE] [get_bd_pins video_subsystem/VGA_DE]
  connect_bd_net -net video_subsystem_VGA_HS [get_bd_ports VGA_HS] [get_bd_pins video_subsystem/VGA_HS]
  connect_bd_net -net video_subsystem_VGA_VS [get_bd_ports VGA_VS] [get_bd_pins video_subsystem/VGA_VS]
  connect_bd_net -net xlslice_0_Dout [get_bd_ports VGA_R] [get_bd_pins xlslice_0/Dout]
  connect_bd_net -net xlslice_1_Dout [get_bd_ports VGA_G] [get_bd_pins xlslice_1/Dout]
  connect_bd_net -net xlslice_2_Dout [get_bd_ports VGA_B] [get_bd_pins xlslice_2/Dout]

  # Create address segments
  create_bd_addr_seg -range 0x40000000 -offset 0x00000000 [get_bd_addr_spaces MNTZorro_v0_1_S00_AXI_0/m00_axi] [get_bd_addr_segs processing_system7_0/S_AXI_HP1/HP1_DDR_LOWOCM] SEG_processing_system7_0_HP1_DDR_LOWOCM
  create_bd_addr_seg -range 0x00010000 -offset 0x43C00000 [get_bd_addr_spaces processing_system7_0/Data] [get_bd_addr_segs MNTZorro_v0_1_S00_AXI_0/S_AXI/reg0] SEG_MNTZorro_v0_1_S00_AXI_0_reg0
  create_bd_addr_seg -range 0x00010000 -offset 0x83000000 [get_bd_addr_spaces processing_system7_0/Data] [get_bd_addr_segs video_subsystem/axi_vdma_0/S_AXI_LITE/Reg] SEG_axi_vdma_0_Reg
  create_bd_addr_seg -range 0x00010000 -offset 0x83C00000 [get_bd_addr_spaces processing_system7_0/Data] [get_bd_addr_segs clk_wiz_0/s_axi_lite/Reg] SEG_clk_wiz_0_Reg
  create_bd_addr_seg -range 0x40000000 -offset 0x00000000 [get_bd_addr_spaces video_subsystem/axi_vdma_0/Data_MM2S] [get_bd_addr_segs processing_system7_0/S_AXI_HP0/HP0_DDR_LOWOCM] SEG_processing_system7_0_HP0_DDR_LOWOCM


  # Restore current instance
  current_bd_instance $oldCurInst

  validate_bd_design
  save_bd_design
}
# End of create_root_design()


##################################################################
# MAIN FLOW
##################################################################

create_root_design ""


