# Makefile for blather project

CFLAGS = -Wall -g
CC     = gcc $(CFLAGS)

PROGRAMS =  bl_server         bl_client \
	    simpio_demo \

all : $(PROGRAMS)

bl_server : bl_server.c server_funcs.c blather.h
	$(CC) -o $@ $^


bl_client : bl_client.c server_funcs.c simpio.c blather.h
	$(CC) -o $@ $^ -lpthread


simpio_demo : simpio_demo.c simpio.c blather.h
	$(CC) -o $@ $^ -lpthread

clean:
	rm -f *.o bl_server bl_client simpio_demo vgcore.* *.fifo *log

include test_Makefile
# which has test targets
