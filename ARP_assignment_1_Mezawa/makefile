all: master drone keyboard watchdog server
	
master:
	gcc master.c -o build/master
drone:
	gcc drone.c -o build/drone
keyboard:
	gcc keyboard.c -o build/keyboard
watchdog:
	gcc watchdog.c -o build/watchdog
server:
	gcc server.c -o build/server -lncurses