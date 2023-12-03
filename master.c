#include <stdio.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include "include/constant.h"


int main(int argc, char *argv[])
{
    // Pids for all children
    pid_t child_watchdog;
    pid_t child_writer0;
    pid_t child_reader0;
    pid_t child_server;

     // Make a log file with the start time/date
    time_t now = time(NULL);
    struct tm *timenow;
    timenow = gmtime(&now);

    char logfile_name[80];

    //There should be a check that the log folder exists but I haven't done that
    sprintf(logfile_name, "./log/watchdog%i-%i-%i_%i:%i:%i.txt", timenow->tm_year + 1900, timenow->tm_mon, timenow->tm_mday, timenow->tm_hour, timenow->tm_min, timenow->tm_sec);

    fopen(PID_FILE_PW, "w");
    char *fnames[NUM_PROCESSES] = PID_FILE_SP;

    for(int i = 0; i < NUM_PROCESSES; i++)
    {
        fopen(fnames[i], "w");
    }

    int res;
    int num_children;

    // Server
    child_server = fork();
    if (child_server < 0) {
        perror("Fork");
        return -1;
    }

    if (child_server == 0) {
        char * arg_list[] = { "konsole", "-e", "./wdserver", "0",NULL };
        execvp("konsole", arg_list);
	return 0;
    }
    num_children += 1;

     // writer 0
    child_writer0 = fork();
    if (child_writer0 < 0) {
        perror("Fork");
        return -1;
    }

    if (child_writer0 == 0) {
        char * arg_list[] = { "./drone2", "1", NULL };
        execvp("./drone2", arg_list);
	return 0;
    }
    num_children += 1;

    // // writer 0
    // child_writer0 = fork();
    // if (child_writer0 < 0) {
    //     perror("Fork");
    //     return -1;
    // }

    // if (child_writer0 == 0) {
    //     char * arg_list[] = { "konsole", "-e", "./drone2", "1", NULL };
    //     execvp("konsole", arg_list);
	// return 0;
    // }
    // num_children += 1;

    // reader 0
    child_reader0 = fork();
    if (child_reader0 < 0) {
        perror("Fork");
        return -1;
    }

    if (child_reader0 == 0) {
        char * arg_list[] = { "konsole", "-e", "./writer", NULL };
        execvp("konsole", arg_list);
	return 0;
    }
    num_children += 1;

    sleep(3);

    // // Create watchdog
    // child_watchdog = fork();
    // if (child_watchdog < 0) {
    //     perror("Fork");
    //     return -1;
    // }

    // if (child_watchdog == 0) {
    //     char * arg_list[] = { "konsole", "-e", "./watchdog", logfile_name, NULL};
    //     execvp("konsole", arg_list);
	// return 0;
    // }
    // num_children += 1;

   // Create wowatchdog
    child_watchdog = fork();
    if (child_watchdog < 0) {
        perror("Fork");
        return -1;
    }

    if (child_watchdog == 0) {
        char * arg_list[] = { "konsole", "-e", "./wowatchdog", logfile_name, NULL};
        execvp("konsole", arg_list);
	return 0;
    }
    num_children += 1;
    
    //wait for all children to terminate
    for(int i = 0; i < num_children; i ++){
        wait(&res);
    }

    return 0;
}

 
