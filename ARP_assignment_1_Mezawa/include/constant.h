#ifndef CONSTANTS_H
#define CONSTANTS_H

#define SHMOBJ_PATH "/shm_server"
#define SEM_PATH "/sem_server"

#define SEED_MULTIPLIER 120
#define MAX_NUMBER 50

#define NUM_PROCESSES 2 /* number of processes watchdog is tracking */
#define PROCESS_SLEEPS_US {100000, 250000} /* time each child process sleeps */
#define PROCESS_SIGNAL_INTERVAL 5 /* number of process cycles before signaling watchdog */
#define WATCHDOG_SLEEP_US 200000 /* watchdog sleep interval */
#define PROCESS_TIMEOUT_S 10 /* time in seconds of process inactivity before watchdog kills all processes */


#define PID_FILE_SP {"/tmp/pid_file0", "/tmp/pid_file1"} /* filenames for simple process pids */
#define PID_FILE_PW "/tmp/pid_filew" /* filename for watchdog process pid */
#define PROCESS_NAMES {"Process 0", "Process 1"} /* process names used for ncurses windows and in log file */

#endif // !CONSTANTS_H

