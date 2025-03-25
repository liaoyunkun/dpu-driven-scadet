import numpy as np

file_path = "timestamp.txt"

try:
    # Read the top 128 lines into a numpy array
    top_lines_array = np.loadtxt(file_path, dtype=int, max_rows=128)

    # Sum the top 128 elements
    total_sum = np.sum(top_lines_array)

    # Print or use the sum as needed
    print("Sum of the top 128 elements:", total_sum)

except FileNotFoundError:
    print(f"File not found: {file_path}")
except Exception as e:
    print(f"An error occurred: {e}")