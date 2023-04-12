#include <stdio.h>
#include <signal.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <sys/resource.h>
#include <utmp.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <errno.h>
#include <sys/wait.h>

void return_system_information(){
    /*
    Return the information about the machine's name, version, release and achitecture 
    The <sys/utsname.h> header shall define the structure utsname which shall include at least the following members:
    sysname, nodename, release, version and machine which gives information about system information
    Documentation source: https://pubs.opengroup.org/onlinepubs/009695399/basedefs/sys/utsname.h.html
    Note: uname here refers to pointers and all the above members are defined as pointer nodes
    */
    struct utsname info_about_system;
    uname(&info_about_system);

    printf("---------------------------------------\n");
    printf("### System Information ###\n");

    /*
    We will need to make sure that non of the node values are NULL otherwise it will crash the system
    */
    if(info_about_system.sysname != NULL && info_about_system.nodename != NULL && info_about_system.version != NULL 
        && info_about_system.release != NULL && info_about_system.machine != NULL){
    printf(" System Name = %s \n", info_about_system.sysname); //System Name
    printf(" Machine Name = %s \n", info_about_system.nodename); //Machine Name
    printf(" Version = %s \n", info_about_system.version); //Version 
    printf(" Release = %s \n", info_about_system.release); //Release
    printf(" Architecture = %s \n", info_about_system.machine); //Architecture
    }
    else{
        printf("Uname pointer node value is NULL. Can't fetch the inormation!");
    }
    printf("---------------------------------------\n");}

int memory_usage(){
    // total memory utilization
    // rusage data structure will allow us to handle all the parts of the CPU
    // to calculate memory effectively
    // Max contributor is from ru_maxrss
    // Note: it's in the integer format
    struct rusage r_usage;
    getrusage(RUSAGE_SELF,&r_usage);
    return (r_usage.ru_maxrss + r_usage.ru_minflt + r_usage.ru_ixrss + \
    r_usage.ru_isrss + r_usage.ru_idrss + r_usage.ru_majflt + r_usage.ru_nswap + r_usage.ru_inblock + \
    r_usage.ru_oublock + r_usage.ru_msgsnd + r_usage.ru_msgrcv + r_usage.ru_nsignals + r_usage.ru_nvcsw + r_usage.ru_nivcsw);}

void error_message(){
    /*
    Return an output on the terminal stating that there was some issue in the code 
    */
    printf("Something went wrong! Please try again later :)\n");}


float get_cpu_usage_for_one_second(float *a){
    /*
    Returns the cpu usage at the exact moment
    reads the proc file and add up everthing to the total
    and returns as the result
    */
    float user, nice, system, idle, iowait, irq, softirq, steal;
    FILE *f = fopen("/proc/stat", "r");
    if (f == NULL){
        error_message();
        return 1;}
    char eee[1024];
    long long total = 0;
    while (fgets(eee, 1024, f) != NULL) {
        if (strncmp(eee, "cpu ", 4) == 0) {
        //addding all the values, note: all are in float
        sscanf(eee, "cpu %f %f %f %f %f %f %f %f", &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
        int idle_prev = (idle) + (iowait);
        int nidle_prev = (user) + (nice) + (system) + (irq) + (softirq);
        int total_prev = idle_prev + nidle_prev;
        total = (double) total_prev - (double) idle_prev;
        if(a[0] == 0) a[0] = total_prev;
        else{
            a[1] = total_prev;
        }
        break;
    }}
    fclose(f);
    return total;}


void graphic_for_cpu(float a, float *b, char *x){
    /*
    It marks the increament/decreament denoted by ||||
    It does that acc. to the relative value
    a single increament(|) denotes 10% increase
    Note: this is for case where none of a, b is zero 
    ie. This is not for the starting case
    */
    float num = (*b - a);
    float den = a;
    float answer = (num/den)*10;
    
    //to mark the absolute value

    //Sticks represents the 10% mark wheres answer represents the relative difference
    if(answer < 0) answer = -answer;
    int sticks = answer;
    if(sticks != 0){
        for(int i = 1; i < sticks; i++){
            strcat(x, "|");
        }
    }
    strcat(x, " ");
}

void graphic_for_cpu_2(float a, char *p, char *x){
    /*
    It marks the increament/decreament denoted by ||||
    It does that acc. to the relative value
    a single increament(|) denotes 10% increase
    Note: this is for case where a, b is zero 
    ie. This is for the starting case
    */

    float answer = a;
    
    //to mark the absolute value

    //Sticks represents the 10% mark wheres answer represents the relative difference
    if(answer < 0) answer = -answer;
    int sticks = answer*10;
    if(sticks != 0){
        for(int i = 1; i < sticks; i++){
            strcat(x, "|");
        }
    }
    strcat(x, " ");
}


float subtract_after_sleep(float *c, float *a, float *s, int zz) {
    /*
    Return the diff of the a and b cpu usages to 
    get the relative differnce betwen the two
    */
    //The program refreshes for T duration
    usleep(zz * 500000); // sleep for half a second
     s[0] = 0;
   
    //Updation of the value takes place here
    //while the program is asleep
    *c = get_cpu_usage_for_one_second(s);

    //returning the realtive value
    float num = 1000*(*c-*a);
    float den = s[1] - s[0];
    float k = num/den;
    return k/10;
}

float find_cpu_usage(int k){
    /*
    Returns the total cpu usage for the system
    It get's total means that you consider the load across all cores 
    and then it take samples at two separate point (seperated by a gap of 1 second)
    in time and compare them making relative to the first sample
    I believe that there will always be 10. They represent the following:
    user, nice,system, idle, iowait, irq, softirq, steal, guest and guest_nice
    We'll be totalling every thing except guest and guest_nice becuase they are
    included in the user and nice value outputs.
    Note: we'll use the absolute value of the total cpu usage because the 
    relative change may be positive or negative.
    */

    //cpu usages at an instance
    float a[2];
    a[0] = 0;
    float c = get_cpu_usage_for_one_second(a);
    float s = get_cpu_usage_for_one_second(a);
    //We'll get the relative difference of the two cpu usages above
    float result = subtract_after_sleep(&c, &s, a, k);

    return result;}


void get_session_info(){
    /*
    The utmp pointer consists of the following fields,
    ut_line for the Device name
    ut_id for the Terminal name
    ut_user for the Username
    ut_host for the Hostname    
    */
    struct utmp *user_info;
    setutent();
    printf("---------------------------------------\n");
    printf("### Sessions/users ###\n");
    while ((user_info = getutent())) {
        if (user_info->ut_type == USER_PROCESS) {
            printf(" %s       %s (%s)\n", user_info->ut_user, user_info->ut_line, user_info->ut_host);}
    }
    free(user_info);
    endutent();}

void get_session_info2(char cpu[2][1024]){
    /*
    The utmp pointer consists of the following fields,
    ut_line for the Device name
    ut_id for the Terminal name
    ut_user for the Username
    ut_host for the Hostname    
    */
    struct utmp *user_info;
    setutent();
    char username[1024] = "---------------------------------------\n### Sessions/users ###\n";
    while ((user_info = getutent())) {
        if (user_info->ut_type == USER_PROCESS) {
            char line[1024];
            sprintf(line, " %s       %s (%s)\n", user_info->ut_user, user_info->ut_line, user_info->ut_host);
            strcat(username, line);
        }
    }
    strcpy(cpu[0], username);
    free(user_info);
    endutent();
}

void cal_per(float a, float b, char *new){
    /*
    It add on the increament/decreament acc. to the relative value 
    diff: relative value of a and b in percentage form
    : when there's a decreament
    # when there's an increament 
    */

    float diff = ((b - a)/b)*100;

    if(diff < 0){
        for(int i = 0; i < -diff; i++){
            //symbol
            strcat(new, ":");
        }
    }

    if(diff > 0){ 
        for(int j = 0; j < diff; j++){
            //symbol
            strcat(new, "#");
        }        
    }
}

void get_cpu_utilization(int N, int T, char array[N][1024], int index, float matrix[N]){  
    /*
    ### For graphic CPU utilization output
	### intended to see the increament and decreament signs
    It's gives the total cpu usage by aall the cores combined
    It's given by in-class formula
    	Representation: (Phys.Used/Tot -- Virtual Used/Tot)
    	where, Phy.Used means used physical usage, Vir Used means used virtual usage
    	and Tot means thte total storage in the CPU
    We are taking N sample in every T seconds which are given in the arguement
    if not given, default N = 10 and T = 1
    	The size can be changed
    The index maintain the index of main itteration 
    The representation of the change is displayed in terms if # for increase ot : for decrease
    */
    struct sysinfo memory_info; 
    float total_mem;
    sysinfo(&memory_info);

    //calc. for the total virtual memory
    float total_memory = memory_info.totalram;
    total_memory += memory_info.totalswap;
    total_memory *= memory_info.mem_unit;
    float total_virtual_memory = total_memory / (1024 * 1024 * 1024);
    
    //calc. for the active physical memory
    float total_memory_physical_used = memory_info.totalram;
    float used_memory_3 = total_memory_physical_used  - memory_info.freeram;
    used_memory_3 *= memory_info.mem_unit;

    //calc. for the active virtual memory
    float total_memory_vir = memory_info.totalram + memory_info.totalswap;
    float used_memory_4 = total_memory_vir - memory_info.freeram - memory_info.freeswap;
    used_memory_4 *= memory_info.mem_unit;

    if (sysinfo(&memory_info) == 0) {
        total_mem = (long long)memory_info.totalram * memory_info.mem_unit;
    } else {
        error_message();
    }

    //All strings which are required
    char phy_mem_string[128];
    char vir_mem_string[128];
    char tot_phy_mem_string[128];
    char tot_vir_mem_string[128];
    char diff_string[128];
    char string[128];

    //simplifcation of the formula
    float phy_mem = used_memory_3 / (1024 * 1024 * 1024);
    matrix[index] = phy_mem;
    float vir_mem = used_memory_4 / (1024 * 1024 * 1024);
    float tot_phy_mem = total_mem/(1024 * 1024 * 1024);
    float tot_vir_mem = total_virtual_memory;

    //differnec between the vlaue of physical usage active from the previous version of it
    float diff = matrix[index] - matrix[index-1];
    float t = diff;

    //to maintain the absolute quantity
    if(diff < 0) diff = - diff;
    
    //conversion of numbers ot strings Note: upto 2 decimals
    sprintf(phy_mem_string, "%.2f", phy_mem);
    sprintf(vir_mem_string, "%.2f", vir_mem);
    sprintf(tot_phy_mem_string, "%.2f", tot_phy_mem);
    sprintf(tot_vir_mem_string, "%.2f", tot_vir_mem);
    sprintf(tot_vir_mem_string, "%.2f", tot_vir_mem);
    sprintf(diff_string, "%.2f", diff);

    //creating a basic structure so that concatination can be done easily

    //first part PHY
    strcat(string, phy_mem_string);
    strcat(string, " GB / ");
    strcat(string, tot_phy_mem_string);
    strcat(string, " GB  -- ");

    //second part VIR
    strcat(string, vir_mem_string);
    strcat(string, " GB / ");
    strcat(string, tot_vir_mem_string);
    strcat(string, " GB");

    char new[5];
    strcpy(new, "");
    char display_string[10];
    strcpy(display_string, "");
    if(diff < 0) diff = -diff;
    strcat(string, "   |");

    if(index >= 1){
    cal_per(matrix[index-1], matrix[index], string);}

    //We'll use this as the base value
    if(index == 0) strcpy(diff_string, "0.00");


    if(t < 0){
    //decreament
    strcat(string, "@");}
    else{
    //increament
    strcat(string, "*");    
    }
    strcat(string, " ");
    strcat(string, diff_string);
    strcat(string, " ");
    strcat(string, "(");
    strcat(string, phy_mem_string);
    strcat(string, ")\n");

    //updating the array with the newly concatinated string
    strcpy(array[index], string);
    }

void number_of_cores() {
    char result[100];  // Create a buffer to hold the concatenated string
    int len = 0;       // Initialize the length to 0

    // Append the first line to the result string
    len += sprintf(result+len, "---------------------------------------\n");

    // Append the second line, which includes the number of cores
    len += sprintf(result+len, "Number of cores: %ld\n", sysconf(_SC_NPROCESSORS_ONLN));

    // Print the result string
    printf("%s", result);
}

void number_of_cores2(char cpu[2][1024]) {
    char result[100];  // Create a buffer to hold the concatenated string
    int len = 0;       // Initialize the length to 0
    strcpy(result, "");
    // Append the first line to the result string

    // Append the second line, which includes the number of cores
    len += sprintf(result+len, "%ld\n", sysconf(_SC_NPROCESSORS_ONLN));

    // Print the result string
    strcpy(cpu[0], result);
}


void get_cpu_utilization_2(int N, int T, char array[N][1024], int index){ 
    /*
    	### For non-graphic CPU utilization output
	    ### Not-intended to see the increament and decreament signs
    It's gives the total cpu usage by aall the cores combined
    It's given by in-class formula
    	Representation: (Phys.Used/Tot -- Virtual Used/Tot)
    	where, Phy.Used means used physical usage, Vir Used means used virtual usage
    	and Tot means thte total storage in the CPU
    We are taking N sample in every T seconds which are given in the arguement
    if not given, default N = 10 and T = 1
    	The size can be changed
    The index maintain the index of main itteration 
    */
    //retracting the information from the memory
    struct sysinfo memory_info; 
    float total_mem;
    sysinfo(&memory_info);

    //calc. for the total virtual memory
    memory_info.totalram += memory_info.totalswap;
    memory_info.totalram *= memory_info.mem_unit;
    float total_virtual_memory = memory_info.totalram / (1024 * 1024 * 1024);
    
    //calc. for the active physical memory
    float used_memory_3 = memory_info.totalram  - memory_info.freeram;
    used_memory_3 *= memory_info.mem_unit;

    //calc. for the active virtual memory
    float total_memory_vir = memory_info.totalram + memory_info.totalswap;
    float used_memory_4 = total_memory_vir - memory_info.freeram - memory_info.freeswap;
    used_memory_4 *= memory_info.mem_unit;

    if (sysinfo(&memory_info) == 0) {
        total_mem = (long long)memory_info.totalram * memory_info.mem_unit;
    } else {
        error_message();
    }

    //All strings which are required
    char phy_mem_string[128];
    char vir_mem_string[128];
    char tot_phy_mem_string[128];
    char tot_vir_mem_string[128];
    char string[128];
    strcpy(string, "");
    //All the numbers which are requied note: all in float form
    float phy_mem = used_memory_3 / (1024 * 1024 * 1024);
    float vir_mem = used_memory_4 / (1024 * 1024 * 1024);
    float tot_phy_mem = total_mem/(1024 * 1024 * 1024);
    float tot_vir_mem = total_virtual_memory;

    //number to string
    sprintf(phy_mem_string, "%.2f", phy_mem);
    sprintf(vir_mem_string, "%.2f", vir_mem);
    sprintf(tot_phy_mem_string, "%.2f", tot_phy_mem);
    sprintf(tot_vir_mem_string, "%.2f", tot_vir_mem);

    //concatination
    strcat(string, phy_mem_string);
    strcat(string, " GB / ");
    strcat(string, tot_phy_mem_string);
    strcat(string, " GB  -- ");
    strcat(string, vir_mem_string);
    strcat(string, " GB / ");
    strcat(string, tot_vir_mem_string);
    strcat(string, " GB\n");
    
    //updating the array with the newly concatinated string
    strcpy(*(array + index), string);
    }

void call_user(int N, int T, int user, int system, int sequence, int graphic)
{
    int mem_usage = memory_usage();
    char info_array[N][1024];
    int memory_fd[2]; // file descriptors for the pipe  
    int session_fd[2]; // file descriptors for the pipe  
    int cpu_fd[2]; // file descriptors for the pipe
    char temp[1024]; 
    int status;
    for(int i = 0; i < N; i++){
        strcpy(info_array[i], "\n");
    }
    char store[N][1024];
    for(int i = 0; i < N; i++){
        strcpy(store[i], "\n");
    }
    for(int i = 0; i < N; i++){
        printf("\033[2J"); // Clear the screen
        printf("\033[%d;%dH", 0, 0); // Move cursor back to top-left corner
        printf("Nbr of samples: %d -- every %d secs\n", N, T);
        printf(" Memory usage: %d kilobytes\n", mem_usage);
        // Create the pipe
        if (pipe(session_fd) == -1) {
        perror("pipe");
        return;}
        //Session/user information part
        /*
        Returns the information about all the user and the sessions  for this machine
        */
        // Memory usage process
        //FORK 2
        // User session information process
        pid_t pid_sess = fork();
        if (pid_sess == 0) {
            // Child process for user session information
            close(session_fd[1]); // close the write end of the pipe
            char buf[1024];
            ssize_t n = read(session_fd[0], buf, sizeof(buf)); // read from the pipe
            printf("%s", buf);
            close(session_fd[0]); // close the read end of the pipe
            exit(0);
        } else if (pid_sess < 0) {
            // Error occurred
            perror("fork");
            exit(1);
        }
        else{
            close(session_fd[0]);
            char cpu[2][1024];
            strcpy(cpu[0], "");
            strcpy(cpu[1], "");
            get_session_info2(cpu);
            char x[1024];
            strcpy(x, "");
            strcpy(x, cpu[0]);
            ssize_t n = write(session_fd[1], x, sizeof(x)); // write to the pipe
            if (n == -1) {
                perror("write");
            }
            close(session_fd[1]); // close the write end of the pipe
        }
        waitpid(pid_sess, &status, 0); // wait for the child process to finish
        // System information part
        /*
        Return the information about the machine's name, version, release and architecture 
        */
        sleep(T);
    }
    return_system_information();
}

void call_system(int N, int T, int user, int system, int sequence, int graphic)
{
    float k2 = 0;
    float *z = &k2;
    char info_array[N][1024];
    for(int i = 0; i < N; i++){
        strcpy(info_array[i], "\n");
    }
        int mem_usage = memory_usage();
    for(int i = 0; i < N; i++){
        if(*z < 0) *z = -*z;
        //Print statements for the headline part
        printf("\033[2J"); // Clear the screen
        printf("\033[%d;%dH", 0, 0); // Cursor goes to top-left
        printf("Nbr of samples: %d -- every %d secs\n", N, T);
        printf(" Memory usage: %d kilobytes\n", mem_usage);
        printf("---------------------------------------\n");
        printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");  

        //Returns the memory used in Phys.Used/Tot -- Virtual Used/Tot format
        
        get_cpu_utilization_2(N, T, info_array, i);
        
        //Itterating through the values of memory usage 
        for(int i = 0; i < N; i++){
            printf("%s", info_array[i]);}

        //Session/user information part
        /*
        Returns the information about all the user and the sessions  for this machine
        */

        //Core number and CPU usage part
        /*
        Returns the number of cores present in our machine
        */
        number_of_cores();
        
        /*
        Returns the total cpu usage for the system
        */
                if(*z < 0) *z = -*z;

        printf(" total cpu use = %.2f%%\n", *z);

        //Updation of the value of CPU usage so that it can be used in next itteration
        
        *z = find_cpu_usage(T);
            if(*z < 0) *z = -*z;
    }

    //System Information part
    /*
    Return the information about the machine's name, version, release and achitecture 
    */
    return_system_information();
}
void call_sequence(int N, int T, int user, int system, int sequence, int graphic)
{
    float k = 0;
    float *z1 = &k;
    float matrix[N];
    int memory_fd[2]; // file descriptors for the pipe  
    int session_fd[2]; // file descriptors for the pipe  
    int cpu_fd[2]; // file descriptors for the pipe
    char temp[1024]; 
    int status;
    // *z1 = find_cpu_usage(T);
    //         if(*z1 < 0) *z1 = -*z1;
    char store[N][1024];
    for(int i = 0; i < N; i++){
        strcpy(store[i], "\n");
    }
    //increament of k to avoid reduplcation  of code
    //acceptance of the command arguements
    char info_array[N][1024];
    for(int i = 0; i < N; i++){
        strcpy(info_array[i], "\n");
    }

    //memory usage for dsiplay stored in variable for later use
    int mem_usage = memory_usage();
    float ke = 0;
    float *z = &ke;

    for(int i = 0; i < N; i++){
        // Create the pipe
        if (pipe(memory_fd) == -1 || pipe(session_fd) == -1 || pipe(cpu_fd) == -1) {
            perror("pipe");
            return;}
        for(int i = 0; i < N; i++){
            strcpy(info_array[i], "\n");
        }
        if(*z < 0) *z = -*z;
        //To show the current itteration number, it goes uptill N-1
        printf("---------------------------------------\n");
        printf(">>> iteration %d\n", i);
        //use of memory_usage
        printf(" Memory usage: %d kilobytes\n", mem_usage);
        if(user == 0){
            printf("---------------------------------------\n");
            if(graphic == 0){
        //FORK 1
        // Memory usage process
        pid_t pid_mem = fork();
        if (pid_mem == 0) {
            close(memory_fd[1]); // close the write end of the pipe
            // Child process for memory usage
            int mem_usage = memory_usage();
            printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
            // Store the value of i as the element at index i in info_array
            // get_cpu_utilization_2(N, T, info_array, i);
            // strcpy(temp, info_array[i]);
            char buf[1024];
            ssize_t n = read(memory_fd[0], buf, sizeof(buf)); // read from the pipe
            for(int k = 0; k < N; k++){
                strcpy(info_array[k], "\n");
            }
            strcpy(info_array[i], buf);
            for(int k = 0; k < N; k++){
                printf("%s", info_array[k]);
            }
            if (n == -1) {
                perror("read");
                exit(1);
            }
            close(memory_fd[0]); // close the read end of the pipe
            exit(0);
        } else if (pid_mem < 0) {
            // Error occurred
            perror("fork");
            exit(1);
        } else {
            // Parent process
            close(memory_fd[0]); // close the read end of the pipe
            get_cpu_utilization_2(N, T, info_array, i);
            strcpy(temp, info_array[i]);
            ssize_t n = write(memory_fd[1], temp, sizeof(temp)); // write to the pipe
            if (n == -1) {
                perror("write");
                exit(1);
            }
            close(memory_fd[1]); // close the write end of the pipe
        }
        }
        else{
            printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n"); 
            for(int i = 0; i < N; i++){
            strcpy(info_array[i], "\n");}
            get_cpu_utilization(N, T, info_array, i, matrix);

            for(int i = 0; i < N; i++){
            printf("%s", info_array[i]);
        }
        }
            
        //Session/user information part
        /*
        Returns the information about all the user and the sessions  for this machine
        */}
        if(system == 0){
        pid_t pid_sess = fork();
        if (pid_sess == 0) {
            // Child process for user session information
            close(session_fd[1]); // close the write end of the pipe
            char buf[1024];
            ssize_t n = read(session_fd[0], buf, sizeof(buf)); // read from the pipe
            printf("%s", buf);
            close(session_fd[0]); // close the read end of the pipe
            exit(0);
        } else if (pid_sess < 0) {
            // Error occurred
            perror("fork");
            exit(1);
        }
        else{
            close(session_fd[0]);
            char cpu[2][1024];
            strcpy(cpu[0], "");
            strcpy(cpu[1], "");
            get_session_info2(cpu);
            char x[1024];
            strcpy(x, "");
            strcpy(x, cpu[0]);
            strcat(x, "--------------------------------------\n");
            ssize_t n = write(session_fd[1], x, sizeof(x)); // write to the pipe
            if (n == -1) {
                perror("write");
            }
            close(session_fd[1]); // close the write end of the pipe
        }
        waitpid(pid_sess, &status, 0);
        }
        //Core number and CPU usage part
        /*
        Returns the number of cores present in our machine
        */
        if(graphic == 0){
        //FORK 3
        // CPU usage process
        pid_t pid_cpu;
        pid_cpu = fork();
        if (pid_cpu == 0) {
            // Child process for CPU usage
            close(cpu_fd[1]); // close the write end of the pipe
            char buf[1024];
            ssize_t n = read(cpu_fd[0], buf, sizeof(buf)); // read from the pipe
            printf(" total cpu usage: %s%%\n", buf);
            exit(0);
        } else if (pid_cpu < 0) {
            // Error occurred
            perror("fork");
            exit(1);
        } else {
            // Parent process
            char cpu[50];
            char x[2][1024];
            strcpy(x[0], "");
            strcpy(x[1], "");
            number_of_cores2(x);
            printf("Number of cores: %s", x[0]);
            float cpu_usage = find_cpu_usage(T);
            if (cpu_usage < 0) {
                cpu_usage = -cpu_usage;
            }
            if (cpu_usage == -0) {
                cpu_usage = 0;
            }
            sprintf(cpu, "%.2f", cpu_usage);
            close(cpu_fd[0]); // close the read end of the pipe
            ssize_t n = write(cpu_fd[1], cpu, sizeof(cpu)); // write to the pipe
            if (n == -1) {
                perror("write");
                exit(1);
            }
            close(cpu_fd[1]); // close the write end of the pipe
        }
        waitpid(pid_cpu, NULL, 0);
        }
        else{
                            if(*z < 0) *z = -*z;
        printf(" total cpu use = %.2f%%\n", *z);

        //set-up for the cpu utilization method
        char storage[1024] = "";
        char value[1024] = "";
        char x[28] = "";
            if(*z <= 0) *z = -*z;
            if(*z == -0) *z = 0;
        strcat(x, "         |");
        sprintf(value, "%.2f", *z);
        strcat(x, storage);

        //Case when i equals 0 so we'll consider it as a base case and will represent the thing by a mark 
        if(i == 0){
            graphic_for_cpu_2(*z, 0, x);
        }
        if(i == 1){
            graphic_for_cpu_2(*z, 0, x);
        }
        
        //Case when i not equals 0 so we'll have the cpu_usage value of previous function
        /*
        Prev represents the cpu usage from previous itteration and z represent of current itteration
        */
                        if(*z < 0) *z = -*z;
                        if(*z == -0) *z = 0;
        if(i > 1){
                if(*z <=0) *z = -*z;
                if(*z == -0) *z = 0;
        graphic_for_cpu_2(*z, 0, x);}

        for(int y = 0; y < i; y++){
        strcpy(store[y], "\n");}

        strcat(x, value);
        strcat(x, "\n");
        strcpy(store[i], x);

        //Itteration through every itteration where cpu usage of every itteration is stored
        for(int y = 0; y <= i; y++){
        printf("%s", store[y]);}
        
        /*
        Returns the total cpu usage for the system
        */
        *z = find_cpu_usage(T);
        }
        usleep(T * 500000); // sleep for half a second`
        }
        return_system_information();
        //sleeping for T seconds
}

void call_graphic(int N, int T, int user, int system, int sequence, int graphic)
{
    float b = 0;
    int memory_fd[2]; // file descriptors for the pipe  
    int session_fd[2]; // file descriptors for the pipe  
    int cpu_fd[2]; // file descriptors for the pipe
    char temp[1024]; 
    int status;
    int mem_usage = memory_usage();
    float *z = &b;
    float matrix[N];
    *z = find_cpu_usage(T);
    if(*z < 0) *z = -*z;
    char info_array[N][1024];
    for(int i = 0; i < N; i++){
        strcpy(info_array[i], "\n");
    }
    char store[N][1024];
    for(int i = 0; i < N; i++){
        strcpy(store[i], "\n");
    }
    for(int i = 0; i < N; i++){
        if (pipe(memory_fd) == -1 || pipe(session_fd) == -1 || pipe(cpu_fd) == -1) {
            perror("pipe");
            return;}
        printf("\033[2J"); // Clear the screen
        printf("\033[%d;%dH", 0, 0); // Move cursor back to top-left corner
        printf("Nbr of samples: %d -- every %d secs\n", N, T);
        printf(" Memory usage: %d kilobytes\n", mem_usage);
        if(user == 0){
        printf("---------------------------------------\n");
        printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");  

        //Returns the memory used in Phys.Used/Tot -- Virtual Used/Tot format
        //FORK 1
        // Memory usage process
        pid_t pid_mem = fork();
        if (pid_mem == 0) {
            close(memory_fd[1]); // close the write end of the pipe
            // Child process for memory usage
            int mem_usage = memory_usage();
            // Store the value of i as the element at index i in info_array
            // get_cpu_utilization_2(N, T, info_array, i);
            // strcpy(temp, info_array[i]);
            char buf[1024];
            ssize_t n = read(memory_fd[0], buf, sizeof(buf)); // read from the pipe
            strcpy(info_array[i], buf);
            for(int k = 0; k < N; k++){
                printf("%s", info_array[k]);
            }
            if (n == -1) {
                perror("read");
                exit(1);
            }
            close(memory_fd[0]); // close the read end of the pipe
            exit(0);
        } else if (pid_mem < 0) {
            // Error occurred
            perror("fork");
            exit(1);
        } else {
            // Parent process
            close(memory_fd[0]); // close the read end of the pipe
            get_cpu_utilization(N, T, info_array, i, matrix);
            strcpy(temp, info_array[i]);
            ssize_t n = write(memory_fd[1], temp, sizeof(temp)); // write to the pipe
            if (n == -1) {
                perror("write");
                exit(1);
            }
            close(memory_fd[1]); // close the write end of the pipe
        }

        for(int i = 0; i < N; i++){
            printf("%s", info_array[i]);}}
        if(user == 1 && system == 1){{
        printf("---------------------------------------\n");
        printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");  

        //Returns the memory used in Phys.Used/Tot -- Virtual Used/Tot format
        get_cpu_utilization(N, T, info_array, i, matrix);

        for(int i = 0; i < N; i++){
            printf("%s", info_array[i]);}}  
        }

        //Session/user information part
        /*
        Returns the information about all the user and the sessions  for this machine
        */
       if(system == 0 && user == 1){
        pid_t pid_sess = fork();
        if (pid_sess == 0) {
            // Child process for user session information
            close(session_fd[1]); // close the write end of the pipe
            char buf[1024];
            ssize_t n = read(session_fd[0], buf, sizeof(buf)); // read from the pipe
            printf("%s", buf);
            close(session_fd[0]); // close the read end of the pipe
            exit(0);
        } else if (pid_sess < 0) {
            // Error occurred
            perror("fork");
            exit(1);
        }
        else{
            close(session_fd[0]);
            char cpu[2][1024];
            strcpy(cpu[0], "");
            strcpy(cpu[1], "");
            get_session_info2(cpu);
            char x[1024];
            strcpy(x, "");
            strcpy(x, cpu[0]);
            ssize_t n = write(session_fd[1], x, sizeof(x)); // write to the pipe
            if (n == -1) {
                perror("write");
            }
            close(session_fd[1]); // close the write end of the pipe
        }
        waitpid(pid_sess, &status, 0);
        }
       if(system == 1 && user == 1){
        pid_t pid_sess = fork();
        if (pid_sess == 0) {
            // Child process for user session information
            close(session_fd[1]); // close the write end of the pipe
            char buf[1024];
            ssize_t n = read(session_fd[0], buf, sizeof(buf)); // read from the pipe
            printf("%s", buf);
            close(session_fd[0]); // close the read end of the pipe
            exit(0);
        } else if (pid_sess < 0) {
            // Error occurred
            perror("fork");
            exit(1);
        }
        else{
            waitpid(pid_sess, NULL, 0);
            close(session_fd[0]);
            char cpu[2][1024];
            strcpy(cpu[0], "");
            strcpy(cpu[1], "");
            get_session_info2(cpu);
            char x[1024];
            strcpy(x, "");
            strcpy(x, cpu[0]);
            strcat(x, "--------------------------------------\n");
            ssize_t n = write(session_fd[1], x, sizeof(x)); // write to the pipe
            if (n == -1) {
                perror("write");
            }
            close(session_fd[1]); // close the write end of the pipe
        }
        waitpid(pid_sess, &status, 0);
        }
        
        //Core number and CPU usage part
        /*
        Returns the number of cores present in our machine 
        */
        number_of_cores();

        /*
        Returns the total cpu usage for the system
        Note: Here | refers to to 10 % change in the value
        Eg, if a number changes from 30 to 60 it's a 100% increase
        which signifies 10 | in the the code
        Note: | is always at the starting which shows the boundary line
        ignore the first |
        */
                if(*z < 0) *z = -*z;
        printf(" total cpu use = %.2f%%\n", *z);

        //set-up for the cpu utilization method
        char storage[1024] = "";
        char value[1024] = "";
        char x[28] = "";
                    if(*z <= 0) *z = -*z;

        strcat(x, "         |");
        sprintf(value, "%.2f", *z);
        strcat(x, storage);

        //Case when i equals 0 so we'll consider it as a base case and will represent the thing by a mark 
        if(i == 0){
            graphic_for_cpu_2(*z, 0, x);
        }
        if(i == 1){
            graphic_for_cpu_2(*z, 0, x);
        }
        
        //Case when i not equals 0 so we'll have the cpu_usage value of previous function
        /*
        Prev represents the cpu usage from previous itteration and z represent of current itteration
        */
                        if(*z < 0) *z = -*z;
        if(i > 1){
                if(*z <=0) *z = -*z;
        graphic_for_cpu_2(*z, 0, x);}

        strcat(x, value);
        strcat(x, "\n");
        strcpy(store[i], x);

        //Itteration through every itteration where cpu usage of every itteration is stored
        for(int y = 0; y < i; y++){
        printf("%s", store[y]);}
        
        /*
        Returns the total cpu usage for the system
        */
        *z = find_cpu_usage(T);
    }

    //System Information part
    /*
    Return the information about the machine's name, version, release and achitecture 
    */
   return_system_information();
}

// void find_memory_usage(int N, int T, int user, int system, int sequence, int graphic){
//     char info_array[N][1024];
//     for(int i = 0; i < N; i++){
//         strcpy(info_array[i], "\n");
//     }
//     int mem_usage = memory_usage();
//     printf("\033[2J"); // Clear the screen
//     printf("\033[%d;%dH", 0, 0); // Cursor goes to top-left
//     printf("Nbr of samples: %d -- every %d secs\n", N, T);
//     printf(" Memory usage: %d kilobytes\n", mem_usage);
//     printf("---------------------------------------\n");
//     printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
// }

void call_for_nothing(int N, int T, int user, int system, int sequence, int graphic, char info_array[N][1024]) {
    char cup_storage[N][1024];
    for(int hg = 0; hg < N; hg++){
        strcpy(info_array[hg], "\n");
    }
    for (int i = 0; i < N; i++) {
        int memory_fd[2]; // file descriptors for the pipe  
        int session_fd[2]; // file descriptors for the pipe  
        int cpu_fd[2]; // file descriptors for the pipe
        char temp[1024]; 
        int status;
        // Create the pipe
        if (pipe(memory_fd) == -1 || pipe(session_fd) == -1 || pipe(cpu_fd) == -1) {
            perror("pipe");
            return;}


        //FORK 1
        // Memory usage process
        pid_t pid_mem = fork();
        if (pid_mem == 0) {
            close(memory_fd[1]); // close the write end of the pipe
            // Child process for memory usage
            int mem_usage = memory_usage();
            printf("\033[2J"); // Clear the screen
            printf("\033[%d;%dH", 0, 0); // Cursor goes to top-left
            printf("Number of samples: %d -- every %d secs\n", N, T);
            printf("Memory usage: %d kilobytes\n", mem_usage);
            printf("---------------------------------------\n");
            printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
            // Store the value of i as the element at index i in info_array
            // get_cpu_utilization_2(N, T, info_array, i);
            // strcpy(temp, info_array[i]);
            char buf[1024];
            ssize_t n = read(memory_fd[0], buf, sizeof(buf)); // read from the pipe
            strcpy(info_array[i], buf);
            for(int k = 0; k < N; k++){
                printf("%s", info_array[k]);
            }
            if (n == -1) {
                perror("read");
                exit(1);
            }
            close(memory_fd[0]); // close the read end of the pipe
            exit(0);
        } else if (pid_mem < 0) {
            // Error occurred
            perror("fork");
            exit(1);
        } else {
            // Parent process
            close(memory_fd[0]); // close the read end of the pipe
            get_cpu_utilization_2(N, T, info_array, i);
            strcpy(temp, info_array[i]);
            ssize_t n = write(memory_fd[1], temp, sizeof(temp)); // write to the pipe
            if (n == -1) {
                perror("write");
                exit(1);
            }
            close(memory_fd[1]); // close the write end of the pipe
        }


        
        //FORK 2
        // User session information process
        pid_t pid_sess = fork();
        if (pid_sess == 0) {
            // Child process for user session information
            close(session_fd[1]); // close the write end of the pipe
            char buf[1024];
            ssize_t n = read(session_fd[0], buf, sizeof(buf)); // read from the pipe
            printf("%s\n", buf);
            close(session_fd[0]); // close the read end of the pipe
            exit(0);
        } else if (pid_sess < 0) {
            // Error occurred
            perror("fork");
            exit(1);
        }
        else{
            waitpid(pid_mem, NULL, 0);
            close(session_fd[0]);
            char cpu[2][1024];
            strcpy(cpu[0], "");
            strcpy(cpu[1], "");
            get_session_info2(cpu);
            char x[1024];
            strcpy(x, "");
            strcpy(x, cpu[0]);
            strcat(x, "--------------------------------------");
            ssize_t n = write(session_fd[1], x, sizeof(x)); // write to the pipe
            if (n == -1) {
                perror("write");
            }
            close(session_fd[1]); // close the write end of the pipe
        }
        
        
        //FORK 3
        // CPU usage process
        pid_t pid_cpu;
        pid_cpu = fork();
        if (pid_cpu == 0) {
            // Child process for CPU usage
            close(cpu_fd[1]); // close the write end of the pipe
            char buf[1024];
            ssize_t n = read(cpu_fd[0], buf, sizeof(buf)); // read from the pipe
            printf(" total cpu usage: %s%%\n", buf);
            exit(0);
        } else if (pid_cpu < 0) {
            // Error occurred
            perror("fork");
            exit(1);
        } else {
            // Parent process
            waitpid(pid_sess, NULL, 0);
            char cpu[50];
            char x[2][1024];
            strcpy(x[0], "");
            strcpy(x[1], "");
            number_of_cores2(x);
            printf("Number of cores: %s", x[0]);
            float cpu_usage = find_cpu_usage(T);
            if (cpu_usage < 0) {
                cpu_usage = -cpu_usage;
            }
            if (cpu_usage == -0) {
                cpu_usage = 0;
            }
            sprintf(cpu, "%.2f", cpu_usage);
            close(cpu_fd[0]); // close the read end of the pipe
            ssize_t n = write(cpu_fd[1], cpu, sizeof(cpu)); // write to the pipe
            if (n == -1) {
                perror("write");
                exit(1);
            }
            close(cpu_fd[1]); // close the write end of the pipe
        }
        waitpid(pid_cpu, NULL, 0);
    // Wait for child processes to finish
    // Sleep for T seconds
    usleep(T * 500000); // sleep for half a second
    }
}

void normal_execution(int N, int T, int user, int system, int sequence, int graphic){
    char info_array[N][1024];
    for(int i = 0; i < N; i++){
        strcpy(info_array[i], "\n");
    }
    call_for_nothing(N, T, user, system, sequence, graphic, info_array);
    return_system_information();    
}

void call_function(int N, int T, int user, int system, int sequence, int graphic){
    /*
    This function is called by the main function
    It's the main function*/
    if(user == 1){
        if(sequence == 0 && system == 0 && graphic == 0){
        call_user(N, T, user, system, sequence, graphic);}
    }
    if(system == 1){
        if(sequence == 0 && user == 0 && graphic == 0){
        call_system(N, T, user, system, sequence, graphic);}
    }
    if(sequence == 1){
        int k = 0;
        if(user == 1 && system == 1) k++;
        if(k == 0){
        call_sequence(N, T, user, system, sequence, graphic);}
        if(k!=0){
            user  = 0;
            system = 0;
            call_sequence(N, T, user, system, sequence, graphic);
            exit(0);
        }
    }
    if(graphic == 1 && sequence != 1){
        call_graphic(N, T, user, system, sequence, graphic);
    }
    if((user == 0 && system == 0 && sequence == 0 && graphic == 0) || (user == 1 && system == 1 && sequence == 0 && graphic == 0)){
        normal_execution(N, T, user, system, sequence, graphic);
    }
}

// Define signal handlers
void sigint_handler(int signum) {
    // Handle the Ctrl-C signal
    // Ask the user if they want to quit
    // If yes, then exit the program
    // If no, then continue the program
    // As per the assignment, we should ask the user if they want to quit
    char choice;
    printf("Are you sure, you want to quit? (y/n) ");
    scanf(" %c", &choice);
    if (choice == 'y' || choice == 'Y' || strcmp(&choice,"yes") == 0 || strcmp(&choice, "Yes") == 0) {
        exit(0);
    }
}

void sigtstp_handler(int signum) {
    // Ignore the Ctrl-Z signal
    // As per the assignment, we should ignore this signal
    printf("Ctrl + Z is ignored\n"); 
    return;
}

int main(int argc, char *argv[])
{   
    // Register signal handlers
    /*
    The below two lines of code are used to handle the signals
    */
    signal(SIGINT, sigint_handler);//SIGINT is the signal for Ctrl-C
    signal(SIGTSTP, sigtstp_handler);//SIGTSTP is the signal for Ctrl-Z

    int N = 10;
    int T = 1;
    
    int user = 0;
    int system = 0;
    int sequence = 0;
    int graphic = 0;

    if(argc > 1){

    for(int i = 0; i < argc; i++){
        if(strncmp(argv[i], "--tdelay=", 9) == 0){
            sscanf(argv[i], "--tdelay=%d", &T);
        }
        if(strncmp(argv[i], "--samples=", 9) == 0){
            sscanf(argv[i], "--samples=%d", &N);
        }
    }}

    if (argc > 1) {
        for(int i = 1; i < argc; i++){
            if (sscanf(argv[i], "%d", &N) == 1){
                if(i+1 < argc && sscanf(argv[i+1], "%d", &T) == 1) break;
            }
        }
    }

    if(argc > 1){
        for(int i = 0; i < argc; i++){
        if(strcmp(argv[i], "--system") == 0) system = 1;
        if(strcmp(argv[i], "--user") == 0) user = 1;
        if(strcmp(argv[i], "--graphics") == 0 || strcmp(argv[i], "-g") == 0) graphic = 1;
        if(strcmp(argv[i], "--sequential") == 0) sequence = 1;
        }
    }
    call_function(N, T, user, system, sequence, graphic);
    return 0;
}
