
#!/usr/bin/python3
import subprocess
import sys
import re
import os
import zipfile
import csv_parser

def check_header(header):
    try:
        result = subprocess.run(["gcc", "-E", "-"], input=f"#include <{header}>\n", capture_output=True, text=True)
        if result.returncode == 0:
            return True
            # print(f"The {header} header exists.")
        else:
            return False
            # print(f"The {header} header does not exist.")
    except FileNotFoundError:
        print("GCC compiler is not installed.")

def update_trace_mode(header_file, new_value):
    # Read the contents of the header file
    with open(header_file, 'r') as file:
        lines = file.readlines()

    # Update the TRACE_MODE value
    new_lines = []
    for line in lines:
        if line.startswith("#define TRACE_MODE"):
            line = f"#define TRACE_MODE {new_value}\n"
        new_lines.append(line)

    # Write the updated contents back to the header file
    with open(header_file, 'w') as file:
        file.writelines(new_lines)

    print(f"Updated TRACE_MODE to {new_value} in {header_file}")

def move_and_rename_file(src_path, dst_path):
    """
    Move a file from the source path to the destination path and rename it.

    Args:
        src_path (str): Path of the source file.
        dst_path (str): Path of the destination file.

    Returns:
        bool: True if file is successfully moved and renamed, False otherwise.
    """
    try:
        os.rename(src_path, dst_path) # Rename the file
        return True
    except OSError:
        return False

def invoke_vitis_hls(cmd_str):
    try:
        result = subprocess.run(cmd_str, shell=True)
        if result.returncode == 0:
            print("Vitis HLS executed successfully.")
        else:
            print("Vitis HLS execution failed.")
    except FileNotFoundError:
        print("Vitis HLS is not installed or not in the system PATH.")

def adjust_gmp_value(header_file_path):
    # Read the content of the C++ header file
    with open(header_file_path, 'r') as file:
        content = file.read()

    # Find the line that contains the GMP definition
    gmp_line = re.search(r'#define GMP \d+', content)

    if gmp_line:
        # Extract the current value of GMP
        current_gmp_value = int(gmp_line.group().split()[-1])

        # Check if "gmp.h" header exists
        if check_header("gmp.h"):
            new_gmp_value = 1  # Set GMP to 1 if "gmp.h" exists
        else:
            new_gmp_value = 0  # Set GMP to 0 if "gmp.h" doesn't exist

        if current_gmp_value != new_gmp_value:
            # Adjust the value of GMP in the header file
            adjusted_content = re.sub(r'#define GMP \d+', f'#define GMP {new_gmp_value}', content)

            # Write the adjusted content back to the header file
            with open(header_file_path, 'w') as file:
                file.write(adjusted_content)

            print(f"GMP value has been adjusted to {new_gmp_value}")
        else:
            print(f"GMP value is already set to {current_gmp_value}")
    else:
        print("GMP definition not found in the header file")

def unzip_file(zip_path):
    extract_path = os.path.dirname(zip_path)
    with zipfile.ZipFile(zip_path, 'r') as zip_ref:
        zip_ref.extractall(extract_path)

# Usage example
def main():
    # Access command-line arguments
    args = sys.argv[1:]  # Exclude the script name itself
    # Process the arguments as needed
    opt_type = 0
    exe_type = 0
    trace_mode = 0
    if len(args) >= 1:
        opt_type = int(args[0])
        if (len(args) >= 2):
            exe_type = int(args[1])
        if (len(args) >= 3):
            trace_mode = int(args[2])
    else:
        print("Insufficient arguments. Usage: python run_vitis_hls.py [opt-type, require] \
                [exe_type, optional, default 0(C-sim only)] \
              [trace_mode, optional, default 0]")
        exit(-1)
    vitis_log_name = "vitis_hls_opt_{}_exe_{}_trace_{}.log".format(opt_type, exe_type, trace_mode)
    header_file_path = "../hls/scadet_v{}.h".format(opt_type+1)
    adjust_gmp_value(header_file_path)
    update_trace_mode(header_file_path, trace_mode)
    project_name = "hls_prj_opt_{}".format(opt_type)
    invoke_vitis_hls("vitis_hls run_hls.tcl {} {} {} -l {}".format(opt_type, exe_type, project_name, vitis_log_name))
    loop_path = "hls_prj_opt_{}/solution1/.autopilot/db/loop_performance_info/loop.zip".format(opt_type)
    process_path = "hls_prj_opt_{}/solution1/.autopilot/db/process_stalling_info/process.zip".format(opt_type)
    unzip_file(loop_path)
    unzip_file(process_path)
    dataflow_monitor = "hls_prj_opt_{}/solution1/sim/verilog/dataflow_monitor.sv".format(opt_type)
    map_data = csv_parser.build_map_from_sv_file(dataflow_monitor, opt_type)
    print(map_data)
    report_src_path = "hls_prj_opt_{}/solution1/sim/report/scadet_v{}_cosim.rpt".format(opt_type, opt_type+1)
    report_dst_path = "sim_reports/scadet_v{}_t{}_cosim.rpt".format(opt_type+1, trace_mode)
    move_and_rename_file(report_src_path, report_dst_path)

if __name__ == "__main__":
    main()