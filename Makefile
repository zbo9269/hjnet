CROSS   = arm-fsl-linux-gnueabi-
CC      = $(CROSS)gcc
hjnet : M287.o socket_manage.o watchdog_manage.o config_manage.o
	$(CC) -o hjnet M287.o socket_manage.o watchdog_manage.o config_manage.o
config_manage.o : config_manage.c
	$(CC) -c config_manage.c
M287.o : M287.c
	$(CC) -c M287.c
socket_manage.o : socket_manage.c socket_manage.h 
	$(CC) -c socket_manage.c
watchdog_manage.o : watchdog_manage.c watchdog_manage.h 
	$(CC) -c watchdog_manage.c 

#test : test.o config_manage.o
#	gcc -o test test.o config_manage.o
#test.o : test.c
#	gcc -c test.c
#config_manage.o : config_manage.c
#	gcc -c config_manage.c

clean :
	rm -rf hjnet *.o test
