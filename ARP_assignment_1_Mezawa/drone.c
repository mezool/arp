#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <math.h>
#include <signal.h>
#include <time.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/select.h>
#include "include/constant.h"

//variables
int F_x = 0;
int F_y = 0;
int send_intx = 1;
int send_inty = 1;
int process_num ;


int main(int argc, char *argv[]) {
    int fifo_fd;
    char *fifo_path = "/tmp/myfifotest";  // pass of FIFO

    // open the FIFO to read
    fifo_fd = open(fifo_path, O_RDONLY);

    if (fifo_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    ssize_t read_bytes;

     // extract process number
    if(argc == 2){
        sscanf(argv[1],"%i", &process_num);
    } else {
        printf("wrong number of arguments\n"); 
        return -1;
    }
// inizialize semaphore and shared memory
    int cells[2] = {0,0};
    int shared_seg_size = (1 * sizeof(cells));

    // create semaphore ids
    sem_t * sem_id = sem_open(SEM_PATH, 0);

    // create shared memory pointer
    int shmfd  = shm_open(SHMOBJ_PATH, O_RDWR, S_IRWXU | S_IRWXG);
    if (shmfd < 0)
    {
        perror("shm_open");
        return -1;
    }
    void* shm_ptr = mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    srandom(process_num * SEED_MULTIPLIER);

    //WD
    //Publish your pid
    pid_t watchdog_pid;
    pid_t my_pid = getpid();

    char *fnames[NUM_PROCESSES] = PID_FILE_SP;

    FILE *pid_fp = fopen(fnames[process_num], "w");
    fprintf(pid_fp, "%d", my_pid);
    fclose(pid_fp);

    //printf("Published pid %d \n", my_pid);
    // Read watchdog pid
    FILE *watchdog_fp = NULL;
    struct stat sbuf;

    /* call stat, fill stat buffer, validate success */
    if (stat (PID_FILE_PW, &sbuf) == -1) {
        perror ("error-stat");
        return -1;
    }
    // waits until the file has data
    while (sbuf.st_size <= 0) {
        if (stat (PID_FILE_PW, &sbuf) == -1) {
            perror ("error-stat");
            return -1;
        }
        usleep(50000);
    }

    watchdog_fp = fopen(PID_FILE_PW, "r");

    fscanf(watchdog_fp, "%d", &watchdog_pid);
    //printf("watchdog pid %d \n", watchdog_pid);
    fclose(watchdog_fp);

    while (1) {
        // read from FIFO
        read_bytes = read(fifo_fd, buffer, sizeof(buffer));

        if (read_bytes > 0) {
            //write(STDOUT_FILENO, buffer, read_bytes);
            //write(STDOUT_FILENO, "\n", 1); 
        } else if (read_bytes == 0) {
            // finish the loop when FIFO is closed
            break;
        } else {
            perror("read");
            break;
        }

// calculate forces
        if (strcmp(buffer, "up_left") == 0){
            F_x = F_x - 1;
            F_y = F_y - 1;
        } else if (strcmp(buffer, "up") == 0 ){
            F_y = F_y - 1;
        } else if (strcmp(buffer, "up_right") == 0 ) {
            F_x = F_x + 1;
            F_y = F_y - 1;
        } else if (strcmp(buffer, "left") == 0 ) {
            F_x = F_x - 1;
        } else if (strcmp(buffer, "right") == 0 ) {
            F_x = F_x + 1;
        } else if (strcmp(buffer, "down_left") == 0 ) {
            F_x = F_x - 1;
            F_y = F_y + 1;
        } else if (strcmp(buffer, "down") == 0 ) {
            F_y = F_y + 1;
        } else if (strcmp(buffer, "down_right") == 0 ) {
            F_x = F_x + 1;
            F_y = F_y + 1;
        } else if (strcmp(buffer, "stop") == 0 ) {
            F_x = 0;
            F_y = 0;
        } 
        //printf("F_x is: %d, F_y is: %d\n", F_x, F_y); // added to debug

     // wait for semaphore
        sem_wait(sem_id);
        // copy cells
        memcpy(cells, shm_ptr, shared_seg_size);
        // if your cell value is less than the other cell value, write a random number
        send_intx = F_x % MAX_NUMBER;
        send_inty = F_y % MAX_NUMBER;
        cells[0] = send_intx;
        cells[1] = send_inty;
        // copy local cells to memory
        memcpy(shm_ptr, cells, shared_seg_size);
        //printf("Wrote %i ,%i to server in cell %i\n", send_intx, send_inty, process_num);
        // post semaphore
        sem_post(sem_id);

        if(kill(watchdog_pid, SIGUSR1) < 0)
            {
                perror("kill");
            }
        //printf("Sent signal to watchdog\n"); // added to debug
        usleep(100000);
    }

    // close FIFO
    close(fifo_fd);
    // clean up
    shm_unlink(SHMOBJ_PATH);
    sem_close(sem_id);
    sem_unlink(SEM_PATH);

    return 0;
}
