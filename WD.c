// MiaのWD
#include <ncurses.h>
#include <string.h>
#include <time.h>
#include "include/constant.h"
#include <signal.h>

#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/time.h>
#include <sys/types.h> 
#include <unistd.h> 

//global variables
pid_t sp_pids[NUM_PROCESSES];
struct timeval prev_ts[NUM_PROCESSES];
int process_data_recieved[NUM_PROCESSES] = {0, 0};
char logfile_name[80];
char *process_names[NUM_PROCESSES] = PROCESS_NAMES; // Names to be displayed


// logs time update to file
void log_receipt(pid_t process_id, char *process_name, struct timeval tv)
{
    FILE *lf_fp = fopen(logfile_name, "a");
    fprintf(lf_fp, "%s [%d]: %ld %ld\n", process_name, process_id, tv.tv_sec, tv.tv_usec);
    fclose(lf_fp);
}

// updates the process data received and previous time
void process_update_handler(int sig, siginfo_t *info, void *context)
{
    for(int i = 0; i < NUM_PROCESSES; i++){
        if(info->si_pid == sp_pids[i]){
            process_data_recieved[i] = 1;
            gettimeofday(&prev_ts[i], NULL);
            log_receipt(sp_pids[i], process_names[i], prev_ts[i]);
        }
    }
}

// creates a window with the correct height, draws a box around it and refreshes
WINDOW *create_newwin(int height, int width, int starty, int startx)
{
    WINDOW *local_win;
    local_win = newwin(height, width, starty, startx);
    box(local_win, 0 , 0);
    /* 0, 0 gives default characters
    * for the vertical and horizontal
    * lines
    */
    wrefresh(local_win);
    /* Show that box */
    return local_win;
}

// Gets elapsed time in seconds between two timevals
double get_elapsed_time_s(struct timeval current, struct timeval previous)
{
    return (double)(current.tv_sec - previous.tv_sec) + (double)(current.tv_usec - previous.tv_usec) / 1000000;
}

// update window text with new elapsed times, retgurns 1 if time elapsed longer than process timeout
int update_window_text(WINDOW **windows)
{
    int window_height;
    int window_width;
    struct timeval read_time;
    gettimeofday(&read_time, NULL);
    double elapsed;

    for(int i = 0; i < NUM_PROCESSES; i++)
    {
        if (process_data_recieved[i])
        {
            getmaxyx(windows[i], window_height, window_width);
            elapsed = get_elapsed_time_s(read_time, prev_ts[i]);
            if(elapsed > PROCESS_TIMEOUT_S)
            {
                wattron(windows[i], COLOR_PAIR(1));
                return -1;
            }
            else
            {
                wattron(windows[i], COLOR_PAIR(3));
            }
            mvwprintw(windows[i], window_height/2, window_width - 20, "Time elapsed: %05.3f", elapsed);
            wrefresh(windows[i]);
        }
        
    }
    return 0;
}

// Terminates all watched processes
void terminate_all_watched_processes()
{
    for(int i = 0; i < NUM_PROCESSES; i ++)
    {
        if(kill(sp_pids[i], SIGKILL) < 0)
        {
            perror("kill");
        }
    }
}

int main(int argc, char *argv[])
{
    //publish the watchdog pid
    pid_t watchdog_pid = getpid();

    FILE *watchdog_fp = fopen(PID_FILE_PW, "w");
    fprintf(watchdog_fp, "%d", watchdog_pid);
    fclose(watchdog_fp); 

    // Reading in pids for other processes
    FILE *pid_fp = NULL;
    struct stat sbuf;

    char *fnames[NUM_PROCESSES] = PID_FILE_SP;

    for(int i = 0; i < NUM_PROCESSES; i++)
    {
//         /* call stat, fill stat buffer, validate success */
//         if (stat (fnames[i], &sbuf) == -1) {
//             perror ("error-stat");
//             return -1;
//         }


        while (sbuf.st_size <= 0) {
            if (stat (fnames[i], &sbuf) == -1) {
                perror ("error-stat");
                return -1;
            }
            usleep(8000);
        }
// Read the PID of each process from the file and store it in sp_pids.
        pid_fp = fopen(fnames[i], "r");

        fscanf(pid_fp, "%d", &sp_pids[i]);

        fclose(pid_fp);
    }

    // Set up sigaction for receiving signals from processes
    struct sigaction p_action;
    p_action.sa_flags = SA_SIGINFO;
    p_action.sa_sigaction = process_update_handler;
    if(sigaction(SIGUSR1, &p_action, NULL) < 0)
    {
        perror("sigaction");
    }

    // Extract log file name
    if(argc == 2){
        snprintf(logfile_name, 80, "%s", argv[1]);
    } else {
        printf("wrong args\n"); 
        return -1;
    }

    // initializes/clears contents of logfile
    FILE *lf_fp = fopen(logfile_name, "w");
    fclose(lf_fp);

    // Initialize Windows
    int window_width, window_height;
    int ch;
    initscr();
    cbreak();
    start_color();

    // Set up color pairs
    // init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);

    // Window sizes for each subwindow
    window_height = (LINES - 1) / NUM_PROCESSES;
    window_width = COLS - 2;

    refresh();

    // Make windows for each process
    WINDOW *windows[NUM_PROCESSES];

    for (int i = 0; i < NUM_PROCESSES; i++)
    {
        windows[i] = create_newwin(window_height, window_width, i * window_height + 1, 1);
        mvwprintw(windows[i], window_height / 2, 2, "%s", process_names[i]);
        wattron(windows[i], COLOR_PAIR(2));
        mvwprintw(windows[i], window_height / 2, window_width - 18, "No Data Received");
        wrefresh(windows[i]);
    }

    // Get a start time to track total run time
    struct timeval process_start_time;
    gettimeofday(&process_start_time, NULL);

    sleep(3);

    while(1)
    {
        // update_window_text returns -1 if a process has timed out
        
        if(update_window_text(windows) < 0)
        {
            terminate_all_watched_processes();

            // Log termination and total elapsed time
            struct timeval termination_time;
            gettimeofday(&termination_time, NULL);
            double elapsed = get_elapsed_time_s(termination_time, process_start_time);
            FILE *lf_fp = fopen(logfile_name, "a");
            fprintf(lf_fp, "Terminated after running for %05.2f seconds\n", elapsed);
            fclose(lf_fp);
            return -1;
        }
        usleep(WATCHDOG_SLEEP_US);
        
    }

    return 0;
}