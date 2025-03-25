#Create output directory and clear contents
# The name to use for the project.
set project_name "parallel_com"

# The part number of the hardware.
set part_number "xcu200-fsgd2104-2-e"

# The language to use in simulation. This can be VHDL, Verilog, or Mixed.
set simulator_language "Mixed"

# The target language to use for synthesis. This can be VHDL or Verilog.
set target_language "Verilog"

# This is the name of the top module. This is not the name of the file that
# contains the top module, but the name of the module itself (in the code).
# This can be set to an empty string to let the tools decide the top module
# automatically.
set top_module "parallel_com"

# This is the name of the top module to use in simulation. This is not the name
# of the file that contains the top module, but the name of the module itself
# (in the code). This can be set to an empty string to let the tools decide the
# top module automatically.
set top_sim_module ""

set outputdir ./vivado_project
file mkdir $outputdir
set files [glob -nocomplain "$outputdir/*"]
file delete -force *.jou
file delete -force *.log
if {[llength $files] != 0} {
    puts "deleting contents of $outputdir"
    file delete -force {*}[glob -directory $outputdir *]; # clear folder contents
} else {
    puts "$outputdir is empty"
}

#Create project
create_project -part $part_number $project_name $outputdir

add_files ./parallel_com.v
add_files -fileset constrs_1 ./timing.xdc

set_property top $top_module [current_fileset]

#launch synthesis
launch_runs synth_1
wait_on_run synth_1
open_run synth_1 -name netlist_1
report_timing_summary -delay_type max -report_unconstrained -check_timing_verbose \
-max_paths 10 -input_pins -file parallel_com_syn_timing.rpt
report_power -file parallel_com_syn_power.rpt
report_utilization -file parallel_com_syn_utilization.rpt -hierarchical

file delete -force *.jou
file delete -force *.log