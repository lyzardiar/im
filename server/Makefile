CFLAGS	= -O2 -Wall -g

all : server
.PHONY : all

server : server.o servtool.o
	gcc $(CFLAGS) -o server server.o servtool.o -lpthread

server.o : server.c servtool.h
servtool.o : servtool.c servtool.h

.PHONY : clean
clean :
	-rm server server.o servtool.o *~
