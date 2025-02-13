#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdbool.h>
#include <sys/sysinfo.h>
#include <sys/resource.h>
#include <string.h>
#include <time.h>
#include <ctype.h> 


void clearScreen(){
    printf("\033[2J"); // Clear;
}

void moveToTopLeft(){
    printf("\033[H"); // Move cursor to the top-left
}


//Helper function that prints a row of cores for printCores
void print_core_row(int num_cores){
  for (int i = 0; i < num_cores; i++){
        printf("+---+ ");
    }
    printf("\n");
    for (int i = 0; i < num_cores; i++){
        printf("|   | ");
    }
    printf("\n");
    for (int i = 0; i < num_cores; i++){
        printf("+---+ ");
    }
    printf("\n");
}   

//Print core information 
void print_cores(int num_cores, float ghz, int row_size){
    printf("v  Number of Cores: %d @ %.2f GHz\n", num_cores, ghz);
    while (num_cores >= 0){
        if (num_cores < row_size){
            print_core_row(num_cores % row_size);
        } else {
            print_core_row(row_size);
        }
        num_cores = num_cores - row_size;
    }
}

//Function that initializes the memory array with both the x-axis and y-axis, as well as empty characters 
void initializeMemoryArray(int samples, char arr[][samples]){
    for (int i = 0; i < 13; i++){
        for (int j = 0; j < samples; j++){
            if (i == 12 && j == 0){
                arr[i][j] = '\0';
            } else if (j == 0){
                arr[i][j] = '|';
            } else if (i == 12){
                arr[i][j] = '_';
            } else{
                arr[i][j] = ' ';
            }
        }
    }
}

//Function that initializes the memory array with both the x-axis and y-axis, as well as empty characters 
void initializeCPUArray(int samples, char arr[][samples]){
    for (int i = 0; i < 11; i++){
        for (int j = 0; j < samples; j++){
            if (i == 10 && j == 0){
                arr[i][j] = '\0';
            } else if (j == 0){
                arr[i][j] = '|';
            } else if (i == 10){
                arr[i][j] = '_';
            } else{
                arr[i][j] = ' ';
            }
        }
    }
}

//Function that updates the CPU array at sample 'sample_number' with 'used_cpu'% cpu utilization
void updateCPUArray(int sample_number, int sample_size, float used_cpu, char arr[][sample_size]){
    int row = 10;
    while (used_cpu > 0){
        row--;
        used_cpu = used_cpu - 10;
    }
    if (row != 10){
        arr[row][sample_number] = ':';
    }
    return;
}

//Function that updates the memory array at sample 'sample_number' with 'used_memory'GB memory utilization
void updateMemoryArray(int sample_number, int sample_size, int total_memory, float used_memory, char arr[][sample_size]){
    float increment = (1.0 * total_memory) / 12;
    int row = 12;
    while (used_memory > 0){
        row--;
        used_memory = used_memory - increment;
    }
    if (row != 12){
        arr[row][sample_number] = '#';
    }
    return;
}

//Prints the CPU graph by looping over the whole CPU array
void printCPUGraph(int samples, float cpu_usage, char arr[][samples]){
    printf("v  Current CPU Usage: %.2f%%\n\n", cpu_usage);

    for (int i = 0; i < 11; i++){
        for (int j = 0; j < samples; j++){
            if (i == 0 && j == 0){
                printf("100%% %c", arr[i][j]);
            } else if (i == 10 && j == 0){
                printf("  0%%  %c", arr[i][j]);
            } else if(j == 0){
                printf("     %c", arr[i][j]);
            } else { 
                printf("%c", arr[i][j]);
            }
        }
        printf("\n");
    }
}

//Prints the memory graph by looping over the whole memory array
void printMemoryGraph(int samples, int total_memory, float used_memory, char arr[][samples]){
    int offset = floor(log10(abs(total_memory))) + 4; //Offset graph based on length of int total_memory
    printf("v  Current Memory Usage: %.2fGB\n\n", used_memory); 

    for (int i = 0; i < 13; i++){
        for (int j = 0; j < samples; j++){
            if (i == 0 && j == 0){
                printf("%dGB %c", total_memory, arr[i][j]);
            } else if (i == 12 && j == 0){
                printf("0GB %c", arr[i][j]);
                for (int k = 0; k < offset - 3; k++){
                    printf(" ");
                }
            } else if(j == 0){
                for (int k = 0; k < offset; k++){
                    printf(" ");
                }
                printf("%c", arr[i][j]);
            } else { 
                printf("%c", arr[i][j]);
            }
        }
        printf("\n");
    }
}

int getTotalRam(){
    struct sysinfo info;
    sysinfo(&info); //Get sysinfo

    return (int) (info.totalram * pow(10, -9)); //Go into totalram field to get total memory
}

float getUsedRam(){
    struct sysinfo info;
    sysinfo(&info);
    int total_ram = getTotalRam(); //Get total ram
    float free_ram = (info.freeram * pow(10, -9)); //Go into the freeram field to get free ram at time of sysinfo call
    float used_ram = (1.0) * total_ram - free_ram; //Get used ram by subtracting total ram and free ram
    return used_ram;
}

float getCPUFreq(){
    FILE *f = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", "r"); //Open file that contains max frequency of CPU
    if (f == NULL){
        fprintf(stderr, "Could not open max_freq file\n");
        exit(1);
    }
    char line[256];
    fgets(line, sizeof(line), f); 
    fclose(f);
    return (strtol(line, NULL, 10) * pow(10, -6));
}


void getCPUTimes(unsigned long *user, unsigned long *nice, unsigned long *system, unsigned long *idle, unsigned long *iowait,
                unsigned long *softirq, unsigned long *steal, unsigned long *guest, unsigned long *guest_nice) {
    FILE *f = fopen("/proc/stat", "r"); //Open file that contains the CPU times
    if (f == NULL) {
       fprintf(stderr, "Error opening /proc/stat\n");
       exit(1);
    }
    fscanf(f, "cpu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu", user, nice, system, idle, iowait, softirq, softirq, steal, guest, guest_nice); //Parse the first line, retrieving all the times
    fclose(f); 
}


double getCPUUsage() {
    unsigned long user1, nice1, system1, idle1, iowait1, softirq1, steal1, guest1, guest_nice1;
    unsigned long user2, nice2, system2, idle2, iowait2, softirq2, steal2, guest2, guest_nice2;

    getCPUTimes(&user1, &nice1, &system1, &idle1, &iowait1, &softirq1, &steal1, &guest1, &guest_nice1); //Get first reading of cpu times
    struct timespec delay; 
    struct timespec req;
    delay.tv_sec = 0;
    delay.tv_nsec = 30000000; 
    nanosleep(&delay, &req); //Delay a small amount of time for a second reading
    getCPUTimes(&user2, &nice2, &system2, &idle2, &iowait2, &softirq2, &steal2, &guest2, &guest_nice2); //Get second reading of cpu times

    unsigned long total1 = user1 + nice1 + system1 + idle1 + iowait1 + softirq1 + steal1 + guest1 + guest_nice1;
    unsigned long total2 = user2 + nice2 + system2 + idle2 + iowait2 + softirq2 + steal2 + guest2 + guest_nice2;

    unsigned long total_diff = total2 - total1;
    unsigned long idle_diff = idle2 - idle1;


    return (double)(total_diff - idle_diff) / total_diff * 100.0; //Return cpu utilization as a percent using formula mentioned in README
}


void updateFlag(char *flag, int *samples, long *t_delay, bool *d_cpu, bool *d_mem, bool *d_core, bool *read_df){
    if (strcmp(flag, "--cores") == 0) { //Read the cores flag, set it's boolean as well as the display flag to true
        *d_core = true;
        *read_df = true;
    } else if (strcmp(flag, "--cpu") == 0){ //Read the cpu flag, set it's boolean as well as the display flag to true
        *d_cpu = true; 
        *read_df = true;
    } else if (strcmp(flag, "--memory") == 0){ //Read the memory flag, set it's boolean as well as the display flag to true
        *d_mem = true;
        *read_df = true;
    } else if (strstr(flag, "--samples=")){ //Read the samples flag, update the value of samples
        char *num = strpbrk(flag, "=");
        num++;
        if (num == NULL){ //Check if a value is passed to samples flag
            fprintf(stderr, "Invalid use of --samples flag, exiting\n");        
            exit(1);
        } else{
            char* copy = num;
            while (*copy){
                if (!isdigit(*copy)) { 
                    fprintf(stderr, "Invalid use of --samples flag, exiting\n");
                    exit(1);
                } 
                copy++; 
            } 
        }
        int sample = strtol(num, NULL, 10);
        if (sample == 0){ //Check if value passed to samples is valid
            fprintf(stderr, "Invalid use of --samples flag, exiting\n");
            exit(1);
        } 
        *samples = sample;
    } else if (strstr(flag, "--tdelay=")){ //Read tdelay flag, update the value of tdelay 
        char *num = strpbrk(flag, "=");
        num++;
        if (num == NULL){ //Check if a value is passed to tdelay flag
            fprintf(stderr, "Invalid use of --tdelay flag, exiting\n");        
            exit(1);
        } else{
            char* copy = num;
            while (*copy){
                if (!isdigit(*copy)) { 
                    fprintf(stderr, "Invalid use of --tdelay flag, exiting\n");
                    exit(1);
                } 
                copy++; 
            } 
        }
        long delay = strtol(num, NULL, 10);
        if (delay == 0){ //Check if the value passed to tdelay is valid
            fprintf(stderr, "Invalid use of --tdelay flag, exiting\n");
            exit(1);
        }
        *t_delay = delay;
    } else{ //Flag is not recognized, terminate the program
        fprintf(stderr, "Unknown flag \"%s\", exiting\n", flag);
        exit(1);
    }
}


int main(int argc, char **argv){
    int samples = 20;
    long t_delay = 500000;
    int ram = getTotalRam();
    bool display_CPU = false;
    bool display_memory = false;
    bool display_cores = false;
    bool readFlag = false;
    bool readDisplayFlag = false;

    for (int i = 1; i < argc; i++){ //Loop through every single command line arguement 
        if ((strlen(argv[i]) >= 2) && argv[i][0] == '-' && argv[i][1] == '-'){ //Check if it contains the flag indicator "--"
            updateFlag(argv[i], &samples, &t_delay, &display_CPU, &display_memory, &display_cores, &readDisplayFlag);
            readFlag = true;
        } else if (readFlag == false && i == 1){ //Check if user inputted a positional arguement for samples
            int user_sample = strtol(argv[i], NULL, 10);
            if (user_sample == 0){
                fprintf(stderr, "Invalid use of arguement \"%s\" (positional arguements must come before flags), exiting...\n", argv[i]);
                exit(1);
            }
            samples = user_sample;
        } else if (readFlag == false && i == 2){ //Check if user inputted a positional arguement for samples
            long user_delay = strtol(argv[i], NULL, 10);
            if (user_delay == 0){
                fprintf(stderr, "Invalid use of arguement \"%s\" (positional arguements must come before flags), exiting...\n", argv[i]);
                exit(1);
            }
            t_delay = user_delay;
        } else { //Command line arguement is not recognized, terminate the program
            fprintf(stderr, "Invalid arguement \"%s\", exiting...\n", argv[i]);
            exit(1);
        }
    }

    if (readDisplayFlag == false){ //If no display flag was mentioned, display all info about the system
        display_CPU = true;
        display_memory = true;
        display_cores = true;
    }

    struct timespec delay;
    struct timespec t2;
    //Convert the microseconds tdelay into nanoseconds for nanosleep function and it's corresponding timespec struct
    if (t_delay > 1000000){
        delay.tv_sec = t_delay / 1000000;
        delay.tv_nsec = (t_delay - (delay.tv_sec * 1000000)) * 1000;
    } else{
        delay.tv_sec = 0;
        delay.tv_nsec = t_delay * 1000;
    }
    //Initialize the arrays for each graph
    char memory_array[13][samples + 1];
    char cpu_array[11][samples + 1];
    initializeMemoryArray(samples + 1, memory_array);
    initializeCPUArray(samples + 1, cpu_array);
    if (display_CPU == true || display_memory == true){ //Loop through each sample if the graphs are to be printed
        for (int i = 1; i < samples + 1; i++){
            clearScreen();
            moveToTopLeft();
            printf("Nbr of samples: %d -- every %ld microSecs (%.3f secs)\n\n", samples, t_delay, t_delay * pow(10, -6));
            if (display_memory == true){ //Display memory graph if it is to be displayed
                float usedRAM = getUsedRam();
                updateMemoryArray(i, samples + 1, ram, usedRAM, memory_array);
                printMemoryGraph(samples + 1, ram, usedRAM, memory_array);
                printf("\n\n");
            }
            if (display_CPU == true){ //Display the CPU graph if it is to be displayed
                double usage = getCPUUsage();
                updateCPUArray(i, samples + 1,  usage, cpu_array);
                printCPUGraph(samples + 1,  usage, cpu_array);
                printf("\n\n");
            }
            nanosleep(&delay, &t2);
        }
    }
    if (display_cores == true){ //Display # of cores and frequency if it is to be displayed
        float freq = (float) getCPUFreq();
        int num_processors = sysconf(_SC_NPROCESSORS_CONF);
        print_cores(num_processors, freq, 4);
    }
    printf("\n\n");
    return 0;
}