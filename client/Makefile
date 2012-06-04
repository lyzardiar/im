CFLAGS	= -O2 -Wall -g

all : client
.PHONY : all

client : client.o clienttool.o
	gcc $(CFLAGS) -o client client.o clienttool.o -lpthread

client.o : client.c clienttool.h
clienttool.o : clienttool.c clienttool.h

.PHONY : clean
clean :
	-rm client client.o clienttool.o *~
