
import zipfile
import os

def unzip_file(zip_path):
    extract_path = os.path.dirname(zip_path)
    with zipfile.ZipFile(zip_path, 'r') as zip_ref:
        zip_ref.extractall(extract_path)

def main():
    opt_type = 0
    loop_path = "hls_prj_opt_{}/solution1/.autopilot/db/loop_performance_info/loop.zip".format(opt_type)
    process_path = "hls_prj_opt_{}/solution1/.autopilot/db/process_stalling_info/process.zip".format(opt_type)
    unzip_file(loop_path)
    unzip_file(process_path)

if __name__ == "__main__":
    main()