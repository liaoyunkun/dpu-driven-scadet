#!/bin/bash
opt_type=$1
trace_mode=$2

# # Check if opt_type is a valid value
# if [[ $opt_type == 0 || $opt_type == 3 || $opt_type == 4 || $opt_type == 5 ]]; then
#     source /tools/Xilinx/Vitis_HLS/2022.1/settings64.sh
#     python run_vitis_hls.py $opt_type 2 $trace_mode
# else
#     echo "Invalid opt_type. Valid values are: 0, 3, 4, 5."
# fi

# list1=(0 3 4 5)
# list2=(0 1 2 3)
list1=(0)
list2=(4 5)
# Iterate over the lists
source /tools/Xilinx/Vitis_HLS/2022.1/settings64.sh
for opt_type in "${list1[@]}"; do
    for trace_mode in "${list2[@]}"; do
        python run_vitis_hls.py $opt_type 2 $trace_mode
    done
done