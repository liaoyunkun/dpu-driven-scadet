
#!/usr/bin/python3
import subprocess

def invoke_vivado(cmd_str):
    try:
        result = subprocess.run(cmd_str, shell=True)
        if result.returncode == 0:
            print("Vivado executed successfully.")
        else:
            print("Vivado execution failed.")
    except FileNotFoundError:
        print("Vivado is not installed or not in the system PATH.")

def main():
    invoke_vivado("vivado -mode batch -source run_prj_parallel_min.tcl")

if __name__ == "__main__":
    main()