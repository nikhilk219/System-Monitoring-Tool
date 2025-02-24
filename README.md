*System Monitoring Tool - Nikhil Kaku*

**About the program:**

This program is a system monitoring tool written in C, designed to provide real-time insights into CPU/Memory utilization, and processor core details. It collects system information using POSIX system calls and reads from procfs and sysfs within linux to extract CPU statistics, memory usage, and processor frequency.

**How to run the program:**

Compilation: Compile by using the command "gcc -Wall -Werror -std=c99 A1.c -lm -o myMonitoringTool" into the terminal.
Positional Arguements and Assumptions:
- CLA Syntax: ./myMonitoringTool [samples [tdelay]] [--memory] [--cpu] [--cores] [--samples=N] [--tdelay=T]
- Default values for samples is 20 and tdelay is 500000 microseconds.
- If the user does not specify any arguements (only enters ./myMonitoringTool), the program will display memory utilization, cpu utilization, and cores with the default values for samples and tdelay.
- The order of which flags are passed do not matter.
Example: ./myMonitoringTool --memory --cpu = ./myMonitoringTool --cpu --memory
- The flags --samples=N and --tdelay=T can be specified as positional arguements in the corresponding order: samples tdelay. These arguements must be the first ones passed to the program (before flags).
Example: ./myMonitoringTool 50 700000 will run the program with 50 samples and 700000 tdelay
- If the user only specifies the samples or tdelay flags, the program will display all information (memory utilization, cpu utilization, and cores).
- If the user repeats the flags samples and/or tdelay, the value of the last specified flag will be used for the program.
Example: ./myMonitoringTool 65 650000 --cores --cpu --tdelay=500000 --samples=30 --samples=55 will run the program displaying cpu utilization and cores with 55 samples and 500000 tdelay (values of samples and tdelay can be overwritten by repeated flags)


**How I solved the problem:**

Parsing
- In order to parse the command line arguements, I utilized boolean variables that represented each display flag (memory graph, cpu graph, cores) and also for determining whether a flag was read or not (in order to make sure that positional arguements are not passed after flags). I also created int/long variables that contained the # of samples and tdelay (defaulted to 20 and 500000 respectively).
- I checked if whether any of the display flag booleans were true and if they were, I would proceed to display the information of that flag after the parsing was finished.

Graphs
- The approach I took the generate the graphs is by utilizing a 2 dimensional array.
- Each row represents some partition of the total utilization of the graph (each partition is 10% in CPU graph while each partiton is 1/12th of the total ram in the memory graph). Each column represents a specific sample number.
- This apporoach allowed me to easily update the graph for each sample as I would just need to index into the appropriate spot in the array. It also allowed me to easily print the graph as I would just need to iterate through the entire array.

Retrieving System Information:

- I accessed the sysinfo struct to retrieve information about ram.
- I accessed the /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq file to retrieve the max frequency of the CPU.
- I accessed the /proc/stat file and parsed the first line of the file to get CPU times and calculate the CPU utilization.
- I used the sysconf and passed in _SC_NPROCESSORS_CONF POSIX to get the total amount of cores configured in the system.

**Overview of functions:**

- void print_core_row(int num_cores);
A helper function to print_cores that prints 'num_cores' cores in a single row on the screen.

- void print_cores(int num_cores, float ghz, int row_size);
Prints 'num_cores' cores on the screen, each row containing 'row_size' cores. Also prints the cores max frequency in gHZ.

- void initializeMemoryArray(int samples, char arr[][samples]);
Initializes the memory array with both the x-axis and y-axis for the memory graph, as well as empty characters in the body of the graph.

- void initializeCPUArray(int samples, char arr[][samples]);
Initializes the CPU array with both the x-axis and y-axis for the CPU graph, as well as empty characters in the body of the graph.

- void updateCPUArray(int sample_number, int sample_size, float used_cpu, char arr[][sample_size]);
Updates the CPU array by inserting 'sample_number' sample with it's corresponding 'used_cpu' CPU utilization at that sample.

- void updateMemoryArray(int sample_number, int sample_size, int total_memory, float used_memory, char arr[][sample_size]);
Updates the memory by inserting 'sample_number' sample with it's corresponding 'used_memory' memory utilization at that sample.

- void printCPUGraph(int samples, float cpu_usage, char arr[][samples]);
Iterates through the CPU array and prints the graph for CPU utilization.

- void printMemoryGraph(int samples, int total_memory, float used_memory, char arr[][samples]);
Iterates through the memory array and prints the graph for memory utilization.

- int getTotalRam();
Accesses sysinfo struct and returns the total available ram of the computer by using the 'totalram' field of the struct.

- float getUsedRam();
Accesses sysinfo struct and returns the amount of ram used by the computer at the call of the function.
It does this by taking the total ram and subtracting it by the amount of freeram, which is acquired by acessing the 'freeram' field of the struct.

- float getCPUFreq();
Returns the CPU's max frequency by accessing the "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq" file that contains the CPU's max frequency.

- void getCPUTimes(unsigned long *user, unsigned long *nice, unsigned long *system, unsigned long *idle, unsigned long *iowait,
                unsigned long *softirq, unsigned long *steal, unsigned long *guest, unsigned long *guest_nice);
A helper function to getCPUUsage() that accesses the "/proc/stat" file. This file contains numbers that identify the amount of time the CPU has spent performing different kinds of work. The function will update each pointer arguement task with it's corresponding time retrieved from this file. 

- double getCPUUsage();
Returns the CPU utilization  at the time of calling this function as a percentage from 0-100%. It does this by calling the helper function getCPUTimes twice with a small time interval in between each call, and uses the formula (Δtotaltime - Δidletime) / Δtotaltime * 100 to calculate total CPU utilization as a percentage. Note that since the "/proc/stat" file contains the times since the kernal was intialized, meaning that 2 readings to this file in a very small time interval will be needed to get real time CPU utilization.

- void updateFlag(char *flag, int *samples, long *t_delay, bool *d_cpu, bool *d_mem, bool *d_core);
A helper function for parsing the command line arguements. Depending on what flag is passed to the function, it will update the info based on what flag that corresponds to (if it is a valid flag).

- int main();
1. First parse the command line arguements using boolean variables for each of display flag specifying whether to display that flag or not.
2. After parsing, declare the timespec struct variables based on what tdelay is specified. Need to declare these structs as the nanosleep function utilized this struct as it's arguements.
3. After parsing, intitialize both the CPU array and memory array with sample size determining the size of these arrays.
4. If either graph is to be displayed, run through a for loop that updates the graph(s) and clears the screen for each taken sample. At the end of the for loop, use nanosleep function to delay the reading of the next sample for tdelay microseconds as specified by the user.
5. If the cores is to be displayed, display the number of cores as well as the max frequency of the cores. 
6. Terminate the program

**Extra Notes:**

- The program assumes that the number of samples will be so that it fits in the terminal width

