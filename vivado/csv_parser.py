import re
import sys

def build_map_from_sv_file(file_path, opt_type):
    if (opt_type > 3):
        P1_STRING = 'grp_p1DecodeSubseqOpt{}_fu'.format(opt_type)
        P2_STRING = 'grp_p2DecodeSubseqOpt{}_fu'.format(opt_type)
        P4_STRING = 'grp_huffSelfSyncDecP4Opt{}_Pipeline_loop_p4_dec_gen_sym_fu'.format(opt_type)
    else:
        P1_STRING = 'grp_p1DecodeSubseq_fu'
        P2_STRING = 'grp_p2DecodeSubseq_fu'
        P4_STRING = 'grp_huffSelfSyncDecP4_Pipeline_loop_p4_dec_gen_sym_fu'
    map_data = {}

    # Read the SystemVerilog file
    with open(file_path, 'r') as file:
        content = file.read()

    # Extract module_intf blocks using regular expression
    module_intf_blocks = re.findall(r"nodf_module_intf\s+(\w+)\s*\([^;]+?\);", content, re.MULTILINE | re.DOTALL)
    # print(module_intf_blocks)
    # Iterate over module_intf blocks
    with open(file_path, 'r') as file:
        line = ''
        for block in module_intf_blocks:
            intf_name = block.strip()
            pivot_str = "assign {}.ap_start".format(block)
            while (not(pivot_str in line)):
                line = file.readline()
            splitted = re.split(r'\.|\=|\s', line)
            splitted = list(filter(None, splitted))
            pname = splitted[-2]
            if(pname.startswith(P1_STRING) or pname.startswith(P2_STRING) or pname.startswith(P4_STRING)):
                map_data[pname] = intf_name.split('_')[-1].strip()
    return map_data

def stage_build_map_from_sv_file(file_path, opt_type):
    S1_STRING = 'grp_scadet_v{}_Pipeline_Load_Tag_List_Loop_fu'.format(opt_type+1)
    S2_STRING = 'grp_scadet_v{}_Pipeline_Load_Cnt_List_Loop_fu'.format(opt_type+1)
    S3_STRING = 'grp_scadet_v{}_Pipeline_Search_Tag_Line_Loop_1_fu'.format(opt_type+1)
    S4_STRING = 'grp_scadet_v{}_Pipeline_Search_Cnt_Line_Loop_For_Evict_fu'.format(opt_type+1)
    S5_STRING = 'grp_scadet_v{}_Pipeline_Search_Min_Cnt_Line_Loop_For_Evict_fu'.format(opt_type+1)
    S6_STRING = 'grp_scadet_v{}_Pipeline_Search_Tag_Line_Loop_For_Evict_fu'.format(opt_type+1)
    map_data = {}

    # Read the SystemVerilog file
    with open(file_path, 'r') as file:
        content = file.read()

    # Extract module_intf blocks using regular expression
    module_intf_blocks = re.findall(r"nodf_module_intf\s+(\w+)\s*\([^;]+?\);", content, re.MULTILINE | re.DOTALL)
    # print(module_intf_blocks)
    # Iterate over module_intf blocks
    with open(file_path, 'r') as file:
        line = ''
        for block in module_intf_blocks:
            intf_name = block.strip()
            pivot_str = "assign {}.ap_start".format(block)
            while (not(pivot_str in line)):
                line = file.readline()
            splitted = re.split(r'\.|\=|\s', line)
            splitted = list(filter(None, splitted))
            pname = splitted[-2]
            if(pname.startswith(S1_STRING) or pname.startswith(S2_STRING)
               or pname.startswith(S3_STRING) or pname.startswith(S4_STRING)
               or pname.startswith(S5_STRING) or pname.startswith(S6_STRING)):
                map_data[pname] = intf_name.split('_')[-1].strip()
    return map_data
if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python script.py <file_path>")
        sys.exit(1)

    file_path = sys.argv[1]
    # map_data = build_map_from_sv_file(file_path)
    # print(map_data)
    # print(list(map_data.keys()))
    # print(list(map_data.values()))
    stage_map_data = stage_build_map_from_sv_file(file_path, 0)
    print(stage_map_data)
    print(list(stage_map_data.keys()))
    print(list(stage_map_data.values()))