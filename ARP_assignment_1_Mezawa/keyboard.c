#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>

int main() {
    int fifo_fd;
    char *fifo_path = "/tmp/myfifotest";  // pass of FIFO

    // make FIFO
    mkfifo(fifo_path, 0666);

    // open FIFO
    fifo_fd = open(fifo_path, O_WRONLY);

    if (fifo_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Set terminal to non-canonical mode
    struct termios term, original_term;
    tcgetattr(STDIN_FILENO, &original_term);
    term = original_term;
    term.c_lflag &= ~(ICANON | ECHO);
    term.c_cc[VMIN] = 1;  // read only one char
    term.c_cc[VTIME] = 0; // without timeout
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    char input;
    ssize_t n;

   
    while (1) {
        fflush(stdout);  // clear the buffer
        printf("Press a key (without Enter) to send the character. Press 'q' to quit.\n");

        // Reading asynchronous input
        n = read(STDIN_FILENO, &input, 1);

        if (n > 0) {
            if (input == 'q') {
                break; // program finish by typing 'q'
            } else if (input == 'w') {
                // drone moves up_left by typing 'w'
                write(fifo_fd, "up_left", sizeof("up_left"));
            } else if (input == 'e') {
                // drone moves up by typing 'e'
                write(fifo_fd, "up", sizeof("up"));
            } else if (input == 'r') {
                // drone moves up_right by typing 'r'
                write(fifo_fd, "up_right", sizeof("up_right"));
            } else if (input == 's') {
                // drone moves left by typing 's'
                write(fifo_fd, "left", sizeof("left"));
            } else if (input == 'f') {
                // drone moves right by typing 'f'
                write(fifo_fd, "right", sizeof("right"));
            } else if (input == 'x') {
                // drone moves down_left by typing 'x'
                write(fifo_fd, "down_left", sizeof("down_left"));
            } else if (input == 'c') {
                // drone moves down by typing 'c'
                write(fifo_fd, "down", sizeof("down"));
            } else if (input == 'v') {
                // drone moves down_right by typing 'v'
                write(fifo_fd, "down_right", sizeof("down_right"));
             } else if (input == 'd') {
                // Input force become 0 by typing 'h'
                write(fifo_fd, "stop", sizeof("stop"));
            }
        }
    }

    // Return the terminal to its original state
    tcsetattr(STDIN_FILENO, TCSANOW, &original_term);

    // close the FIFO
    close(fifo_fd);

    return 0;
}
