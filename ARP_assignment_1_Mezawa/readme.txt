First ARP Assignment

Here are my first assignments.
It includes :
readme.txt (this txt file)
master.c
drone.c
server.c
watchdog.c
keyboard.c
constant.h
makefile
arp_architecture.jpg

constant.h are in: include


Role of codes:
    The master.c forks and executes each file.
    The keyboard.c assigns the role of each key. It also prompts for key input using konsole and sends the typed key information to the drone.c using FIFO.
    The drone.c calculates the input force by using the information about the typed key from the keyboard.c using FIFO. When this code does this, a signal is sent to the watchdog.c to confirm this code is alive. The drone.c sends the force information to the server.c by using shared memory. To avoid the deadlock, semaphore is also used. 
    The server.c calculates the drone's position by using the force information from the drone.c every 1 second by using shared memory. To avoid the deadlock, semaphore is also used. The signal is sent to the watchdog.c to confirm this code is alive. The server.c manages a blackboard that shows the drone, and this also manages an inspection window that shows the drone's position and input force. This information is written in a log file.
    The watchdog.c confirms whether codes are alive. This terminates the system if the watchdog doesn't receive the signal from each code longer than a certain time (The initial is set to 10 seconds).

To run:
cd ARP_assignment_1_Mezawa
make
cd build
./master

log files are in:
build/log

Role of keys:
Typing 'q', the system is finished.
Typing 'w', the force input to the upper left.
Typing 'e', the force input to the upper.
Typing 'r', the force input to the upper right.
Typing 's', the force input to the left.
Typing 'f', the force input to the right.
Typing 'x', the force input to the lower left.
Typing 'c', the force input to the lower.
Typing 'v', the force input to the lower right.
Typing 'd', the force input to the drone stops.

attention:
    Occasionally some system crashes when the system is booted. If this happens, please try again.
    Since the drone.c sends a signal when a key is typed, leaving the key unpressed will cause the system to be terminated by the watchdog. Please make sure to type regularly. This can also be done as debugging for the watchdog.

These files (including this readme.txt) are shared in the GitHub.
https://github.com/mezool/arp