############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2019 Xilinx, Inc. All Rights Reserved.
############################################################

if {[llength $argv] < 3} {
    puts "Error: Insufficient command line arguments!"
    puts "Usage: run_hls.tcl <opt_type> <hls_exec> <project_name>"
    exit 1
}

set opt_type [lindex $argv 0]
set hls_exec [lindex $argv 1]
set project_name [lindex $argv 2]

if {$opt_type == 0} {
open_project -reset $project_name
set_top scadet_v1 
############################################################
# dut files
############################################################
add_files ../hls/scadet_v1.h 
add_files ../hls/scadet_v1.cpp
} elseif {$opt_type == 1} {
open_project -reset $project_name
set_top scadet_v2 
############################################################
# dut files
############################################################
add_files ../hls/scadet_v2.h 
add_files ../hls/scadet_v2.cpp
} elseif {$opt_type == 3} {
open_project -reset $project_name
set_top scadet_v4 
############################################################
# dut files
############################################################
add_files ../hls/scadet_v4.h 
add_files ../hls/scadet_v4.cpp
} elseif {$opt_type == 4} {
open_project -reset $project_name
set_top scadet_v5 
############################################################
# dut files
############################################################
add_files ../hls/scadet_v5.h 
add_files ../hls/scadet_v5.cpp
} elseif {$opt_type == 5} {
open_project -reset $project_name
set_top scadet_v6 
############################################################
# dut files
############################################################
add_files ../hls/scadet_v6.h 
add_files ../hls/scadet_v6.cpp
} else {
	error -code 1 "Unsupported opt_type. Exiting..."
}

############################################################
# testbench files
############################################################
add_files -tb /home/yunkunliao/scadet_fpga/tb/timestamp.txt

if {$opt_type == 0} {
add_files -tb /home/yunkunliao/scadet_fpga/tb/scadet_v1_tb.cpp
} elseif {$opt_type == 1} {
add_files -tb /home/yunkunliao/scadet_fpga/tb/scadet_v2_tb.cpp
} elseif {$opt_type == 3} {
add_files -tb /home/yunkunliao/scadet_fpga/tb/scadet_v4_tb.cpp
} elseif {$opt_type == 4} {
add_files -tb /home/yunkunliao/scadet_fpga/tb/scadet_v5_tb.cpp
}  elseif {$opt_type == 5} {
add_files -tb /home/yunkunliao/scadet_fpga/tb/scadet_v6_tb.cpp
} else {
	error -code 1 "Unsupported opt_type. Exiting..."
}
############################################################
# in/output data files
############################################################

# Create a solution
open_solution "solution1" -flow_target vivado
# Define technology and clock rate
set_part {xcu200-fsgd2104-2-e}
create_clock -period 4 -name default
# config_dataflow -default_channel fifo -fifo_depth 4096
# config_sdx -target none
# #config_rtl -prefix "prefix_"
# config_rtl -encoding onehot -kernel_profile=0 -module_auto_prefix=0 -mult_keep_attribute=0 -reset control -reset_async=0 -reset_level high -verbose=0
# set_clock_uncertainty 12.5%

# Set variable to select which steps to execute
csim_design -ldflags {-z stack-size=10485760}
# csim_design 
# Set any optimization directives
# End of directives
if {$hls_exec == 1} {
	# Run Synthesis and Exit
	csynth_design
	
} elseif {$hls_exec == 2} {
	# Run Synthesis, RTL Simulation and Exit
	csynth_design
	# cosim_design -trace_level all
	cosim_design -trace_level all -enable_dataflow_profiling -ldflags {-z stack-size=10485760}
} elseif {$hls_exec == 3} { 
	# Run Synthesis, RTL implementation and Exit
	csynth_design
	cosim_design -trace_level all -enable_dataflow_profiling -ldflags {-z stack-size=10485760}
	export_design -flow impl -rtl verilog -format ip_catalog
} else {
	# Default is to exit after setup
}
#cosim_design -trace_level all -argv {-p ../../../../../build/rdo -n 1}
#export_design -format ip_catalog
#export_design -flow impl -rtl verilog -format ip_catalog
#export_design -flow syn -rtl verilog -format syn_dcp 

exit
