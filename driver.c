#include <sys/sysinfo.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <math.h>
#include <utmp.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>

void getCores(){
    long processorNumber = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Number of processors: %ld\n", processorNumber);
}

void getSystemMemory(){
    struct sysinfo information;
    sysinfo(&information);

    long usedMemory = information.totalram - information.freeram;
    printf("%lf GB / %lf GB || %lf GB / %lf GB \n",(usedMemory*information.mem_unit)/pow(1024,3), (information.totalram*information.mem_unit)/pow(1024,3),(usedMemory*information.mem_unit + (information.totalswap-information.freeswap))/pow(1024,3), (information.totalram*information.mem_unit + information.totalswap)/pow(1024,3));
}

int getSystemUsers(){
    struct utmp *information;
    int userNumber = 0;
    setutent();
    do {
        information = getutent();
        if(information == NULL){
            break;
        }
        if(information->ut_type == 7){
            userNumber++;
            printf("Username: %s,    User Info: %s, Host: %s\n", information->ut_user, information->ut_line, information->ut_host);
        }
    }
    while (information != NULL);
    return userNumber;
}

int getTotalCPU(){
    FILE *file = fopen("/proc/stat", "r");
    int info[10];
    fscanf(file,"cpu %d %d %d %d %d %d %d %d %d %d", &info[0], &info[1], &info[2], &info[3], &info[4], &info[5], &info[6], &info[7], &info[8], &info[9]);
   
    int totalTime = info[0] + info[1] + info[2] + info[3] + info[4] + info[5] + info[6] + info[7] + info[8] + info[9];
    fclose(file);
    return totalTime;
}

int getIdleCPU(){
    FILE *file = fopen("/proc/stat", "r");
    int info[4];
    fscanf(file,"cpu %d %d %d %d",&info[0], &info[1], &info[2], &info[3]);
    
    fclose(file);
    return info[3];
}

void getCpuUsage(int previousIdle, int previousTotal, int delay){
    sleep(delay);
    int currentIdle = getIdleCPU();
    int currentTotal = getTotalCPU();
    
    double toSubtract = (double)(currentIdle - previousIdle)/(double)(currentTotal - previousTotal);
    double cpuUsed = (1 - toSubtract)*100;

    printf("%f%%\n",cpuUsed);
}

void getSystemInfo(){
    struct utsname information;
    uname(&information);
    printf("-----SYSTEM INFORMATION-----\n");
    printf("System OS: %s\n",information.sysname);
    printf("Machine Name: %s\n",information.nodename);
    printf("Version: %s\n",information.version);
    printf("Release: %s\n",information.release);
    printf("Architecture: %s\n",information.machine);
}

void standardBehavior(int sample, int delay){
    //Print System Specs
    printf("============================================\n");
    getSystemInfo();

    printf("============================================\n");
    getCores();

    printf("============================================\n");
    printf("CPU Usage:\n");
    for(int k = 0; k<sample; k++){                  //allocates empty lines for memory data
        printf("\n");
    }
    printf("Memory: (Phys.Used/Tot -- Virtual Used/Tot)\n");
    for(int l = 0; l<sample; l++){                  //allocates empty lines for CPU data
        printf("\n");
    }
    printf("\x1b[%dA",(2*sample) + 1);              //jumps up to CPU data
    for(int i = 0; i < sample; i++){
        int previousIdle = getIdleCPU();
        int previousTotal = getTotalCPU();
        getCpuUsage(previousIdle, previousTotal, delay);
        printf("\x1b[%dB",sample);                  //jump down to Memory Data
        getSystemMemory();
        printf("\x1b[%dB\x1b[J",sample-i);
        int userNumber = getSystemUsers();
        printf("\x1b[%dA", userNumber+(2*sample+1)-i);
    }
    printf("\x1b[%dB", sample+2);
    getSystemUsers();
}

void getSystemUsageOnly(int sample, int delay){
    printf("============================================\n");
    printf("CPU Usage:\n");
    for(int k = 0; k<sample; k++){                    //allocates empty lines for memory data
        printf("\n");
    }
    printf("Memory: (Phys.Used/Tot -- Virtual Used/Tot)\n");
    for(int l = 0; l<sample; l++){                    //allocates empty lines for CPU data
        printf("\n");
    }
    printf("\x1b[%dA",(2*sample) + 1);                //jumps up to CPU data

    for(int i = 0; i < sample; i++){
        int previousIdle = getIdleCPU();
        int previousTotal = getTotalCPU();
        getCpuUsage(previousIdle, previousTotal, delay);
        printf("\x1b[%dB",sample);                    //jump down to Memory Data
        getSystemMemory();
        printf("\x1b[%dA",(sample+1));
    }
    printf("\x1b[%dB", sample+1);
}

void getUserInfoOnly(int sample){
     for(int i = 0; i < sample; i++){
        int userNumber = getSystemUsers();
        printf("\x1b[%dA", userNumber-i);
    }
    printf("\x1b[%dB", sample+2);
    getSystemUsers();
}

void getSequentialOuput(int sample, int delay, int system, int user, int specs){
    //Print System Specs
    if(specs == 1){
    printf("============================================\n");
    getSystemInfo();

    printf("============================================\n");
    getCores();
    }

    if(system == 1){
        printf("============================================\n");
        printf("CPU Usage:\n");
        for(int i = 0; i < 10; i++){
            int previousIdle = getIdleCPU();
            int previousTotal = getTotalCPU();
            getCpuUsage(previousIdle, previousTotal, delay);
        }
    
        printf("Memory: (Phys.Used/Tot -- Virtual Used/Tot)\n");
        for(int l = 0; l<10; l++){                  //allocates empty lines for CPU data
        getSystemMemory();
        }
    }
    if(user == 1){
        getSystemUsers();
    }
}

int main(int argc, char **argv){
    int c;
    int system = 0;
    int user = 0;
    int graphics = 0;
    int sequential = 0;
    int sample = 10;
    int delay = 1;

    //checks for positional arguments
    if(argc > 1 && argv[1][0] != '-'){
       sample = strtol(&argv[1][0], NULL, 10);
    }

    if(argc > 2 && argv[2][0] != '-'){
       delay = strtol(&argv[2][0], NULL, 10);
    }

    //parses the flags specified by the user
    while(true){
        int option_index = 0;
        static struct option long_options[] = {
                   {"system", no_argument, NULL, 0},
                   {"user",  no_argument, NULL, 0},
                   {"graphics", no_argument, NULL, 0},
                   {"sequential", no_argument, NULL, 0},
                   {"samples", required_argument, NULL, 0},
                   {"tdelay", required_argument, NULL, 0}
                };

        c = getopt_long(argc, argv, "",long_options, &option_index);
        
        if (c == -1){break;}

        switch (option_index) {
            case 0:                             //get only memory & cpu data
                system = 1;
                break;
            case 1:                             //get only user data
                user = 1;
                break;
            case 2:
                graphics = 1;
                break;
            case 3:
                sequential = 1;
                break;
            case 4:
                sample = strtol(optarg, NULL, 10);
                break;
            case 5:
                delay = strtol(optarg, NULL, 10);
                break;
            default:
                printf("this is not supposed to happen");
            }
        }
        if(system+user+graphics+sequential == 0){
            standardBehavior(sample, delay);
        }

        if((system == 1) && (user+graphics+sequential == 0)){
            getSystemUsageOnly(sample, delay);
        }

        if((user == 1) && (system+graphics+sequential == 0)){
            getUserInfoOnly(sample);
        }

        if(sequential == 1){
            if(system+user == 0){
                getSequentialOuput(sample, delay, 1, 1, 1);
            } else if (system == 1 && user == 0){
                getSequentialOuput(sample, delay, 1, 0, 0);
            } else if (system == 0 && user == 1){
                getSequentialOuput(sample, delay, 0, 1, 0);
            } else {
                getSequentialOuput(sample, delay, 1, 1, 0);
            }
        }
    return 0;
}
