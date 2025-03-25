import numpy as np
import matplotlib.pyplot as plt
from palettable.cartocolors.qualitative import Pastel_10
color_map = Pastel_10.hex_colors
plt.rcParams['font.family'] = 'Arial'
from matplotlib.colors import LinearSegmentedColormap
cmap = LinearSegmentedColormap.from_list('black_white_gradient', ['#000000', '#ffffff'], N=4)
# Define a dictionary to store data
data_dict = {}

# Read and process the file
with open('sum_reports.txt', 'r') as file:
    for line in file:
        # Skip comment lines
        if line.startswith('#'):
            continue

        # Split the line into elements
        elements = line.split()

        # Check if the line has five elements
        if len(elements) == 5:
            key = elements[0]
            index = elements[1]
            values = list(map(int, elements[2:]))

            # Check if the key is already in the dictionary
            if key not in data_dict:
                data_dict[key] = []

            # Append the values to the dictionary
            data_dict[key].append(values)
        else:
            Exception("Invalid line!")
    # for key, values_list in data_dict.items():
    #     data_dict[key] = np.transpose(np.array(values_list))
    for key, values_list in data_dict.items():
        data_dict[key] = np.array(values_list)
        print(f"Key: {key}")
        print(values_list)
    # Extracting average latency data for plotting
    keys = list(data_dict.keys())
    for key in keys:
        print(data_dict[key][:, 1])
    average_latencies = np.array([data_dict[key][:, 1] for key in keys])
    print(average_latencies)

    # Define bar positions and width
    bar_width = 0.15
    bar1_positions = np.arange(len(average_latencies[0, :]))
    bar0_positions = [x - bar_width for x in bar1_positions]
    bar2_positions = [x + bar_width for x in bar1_positions]
    bar3_positions = [x + bar_width for x in bar2_positions]
    # Plotting the bar plot for average latency
    fig, ax = plt.subplots(figsize=(8, 3))

    # ax.bar(bar0_positions, average_latencies[0, :], width=bar_width, label = "Version 0", color = "#3b4992")
    # ax.bar(bar1_positions, average_latencies[1, :], width=bar_width, label = "Version 1", color = "#ee0000")
    # ax.bar(bar2_positions, average_latencies[2, :], width=bar_width, label = "Version 2", color = "#008b45")
    # ax.bar(bar3_positions, average_latencies[3, :], width=bar_width, label = "Version 3", color = "#631879")
    print("*"*10)
    print(average_latencies[0, :])
    print("*"*10)
    print(average_latencies[1, :])
    print("*"*10)
    print(average_latencies[2, :])
    print("*"*10)
    print(average_latencies[3, :])
    print("*"*10)
    ax.bar(bar0_positions, average_latencies[0, :], width=bar_width, label = "Base", color=cmap(0), edgecolor='black')
    ax.bar(bar1_positions, average_latencies[1, :], width=bar_width, label = "+FC", color=cmap(0.25), edgecolor='black')
    ax.bar(bar2_positions, average_latencies[2, :], width=bar_width, label = "+FC,+PM", color=cmap(0.5), edgecolor='black')
    ax.bar(bar3_positions, average_latencies[3, :], width=bar_width, label = "+FC,+PM,+PMF", color=cmap(0.75), edgecolor='black')
    plt.axhline(y=94, color="red", linestyle='--', label='Minimum Inter-request Latency')
    # Configure plot
    ax.set_xticks(bar1_positions)
    ax.set_xticklabels(["Trace 1", "Trace 2", "Trace 3"])
    ax.set_xlabel('Trace Groups', fontsize=12)
    ax.set_ylabel('Average Per Trace Latency\n (Clock Cycles)', fontsize=12)
    plt.tick_params(axis='x', labelsize = 12)
    plt.tick_params(axis='y', labelsize = 12)
    # ax.set_title('Average Latency Analysis')
    plt.legend(fontsize=10, frameon=False, loc="best", ncol=1, fancybox=True, shadow=True)

    # move ax up for 10%
    box1 = ax.get_position()
    ax.set_position([box1.x0 + box1.width * 0.1, box1.y0 + box1.height * 0.1, 
                  box1.width * 0.9, box1.height * 0.9])

    # Save the plot as a PDF file
    plt.savefig('average_latency_analysis.pdf')

    # Show the plot
    plt.show()    

# Print the analyzed data
