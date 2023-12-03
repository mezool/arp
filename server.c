//shserverにncursesを加えたもの
#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <sys/select.h>
#include <unistd.h> 
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include "include/constant.h"
#include <semaphore.h>
#include <sys/mman.h>
#include <ncurses.h>


int main(int argc, char *argv[]) 
{
    int process_num;
    if(argc == 2){
        sscanf(argv[1],"%d", &process_num);  
    } else {
        printf("wrong args\n"); 
        return -1;
    }

    pid_t watchdog_pid;
    // Publish your pid
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
    printf("watchdog pid %d \n", watchdog_pid);
    fclose(watchdog_fp);

    //Read how long to sleep process for
    int sleep_durations[3] = PROCESS_SLEEPS_US;
    int sleep_duration = sleep_durations[process_num];

    int counter;

    // initialize semaphore
    sem_t * sem_id = sem_open(SEM_PATH, O_CREAT, S_IRUSR | S_IWUSR, 1);
    sem_init(sem_id, 1, 0); //initialized to 0 until shared memory is instantiated

    int cells[2] = {0, 0};
    int shared_seg_size = (1 * sizeof(cells));
    int F_x = 0;
    int F_y = 0;
    int M = 1;
    int K = 1;
    double T = 1;

    
    initscr(); //initialize ncurses
    start_color(); // activate the usage of colors
    init_pair(1, COLOR_BLUE, COLOR_BLACK); // red text and black background
    noecho(); //Do not display when a key is entered
    curs_set(0);//Do not display the cursor
    timeout(0); // Unblocked in 0 milisecond
    
    
    int row, col;
    getmaxyx(stdscr, row, col);  // 画面の行数と列数を取得
    double x_i = col/2;
    double x_i1 = col/2;
    double x_i2 = col/2;
    double y_i = row/2;
    double y_i1 = row/2;
    double y_i2 = row/2;

    // create shared memory object
    int shmfd  = shm_open(SHMOBJ_PATH, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG);
    if (shmfd < 0)
    {
        perror("shm_open");
        return -1;
    }
    // truncate size of shared memory
    ftruncate(shmfd, shared_seg_size);
    // map pointer
    void* shm_ptr = mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    // copy initial data into cells
    // memcpy(shm_ptr, cells, shared_seg_size);
    // post semaphore
    sem_post(sem_id); //ここでsemaphoreがOKになる

    // every two seconds print the current values in the server (for debugging)
    while (1) 
    {   
        erase();   // ウィンドウをクリア
        int ch = getch(); //wait for key input
        if (ch == 'q') break; // terminate if "q" is input
        attron(COLOR_PAIR(1)); // 色のペア番号1を有効にする
        mvprintw(y_i, x_i, "+"); // 画面上の指定された位置に線を表示
        attroff(COLOR_PAIR(1)); // 色のペア番号1を無効にする
        //mvprintw(y_i+1, x_i+1, "Debug Info");
        WINDOW *win = newwin(row, col, 0, 0);  // 新しいウィンドウの作成
        box(win, 0, 0);  // ウィンドウに線を描画
        wrefresh(win);
        refresh(); // 画面を更新します。


        sem_wait(sem_id);
        memcpy(cells, shm_ptr, shared_seg_size);
        //共有メモリからデータを読み取り、それを配列 cells にコピーします。
        sem_post(sem_id);
        F_x = cells[0];
        F_y = cells[1];        
        x_i2 = x_i1;
        x_i1 = x_i;
        x_i = ( F_x*T*T +M*(2*x_i1-x_i2)+K*T*x_i1) / (M+K*T);
        if (x_i > col-1){
            x_i = col-1;
        } else if (x_i < 0){
            x_i = 0;
        }
        y_i2 = y_i1;
        y_i1 = y_i;
        y_i = ( F_y*T*T +M*(2*y_i1-y_i2)+K*T*y_i1) / (M+K*T);
        if (y_i > row-1){
            y_i = row-1;
        } else if (y_i < 0){
            y_i = 0;
        }
        //printf("x_i is: %f, y_i is: %f", x_i, y_i);
         // send signal to watchdog every process_signal_interval
         // kill 関数を使用して SIGUSR1 シグナルをウォッチドッグプロセスに送信します。
         
        if(kill(watchdog_pid, SIGUSR1) < 0)
        {
            perror("kill");
        }
        //printf("Sent signal to watchdog\n");  //add to debug
        sleep(2);
    } 

    // clean up
    shm_unlink(SHMOBJ_PATH);
    sem_close(sem_id);
    sem_unlink(SEM_PATH);
    munmap(shm_ptr, shared_seg_size);
    endwin();

    return 0; 
} 