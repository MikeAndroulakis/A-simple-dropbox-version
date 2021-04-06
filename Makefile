OBJS	=server.o functions.o
OBJS2	=client.o functions.o
SOURCE	=server.c client.c functions.c
HEADER	=functions.h structures.h
OUT	=dropbox_server
OUT2	=dropbox_client
CC	=gcc
FLAGS	=-c
FLAGS2	=-lpthread

all: $(OBJS) $(OBJS2)
	$(CC) -g $(OBJS) -pthread -o $(OUT)
	$(CC) -g $(OBJS2) -pthread -o $(OUT2)

server.o: server.c
	$(CC) $(FLAGS) server.c

client.o: client.c
	$(CC) $(FLAGS) client.c $(FLAGS2)

functions.o: functions.c
	$(CC) $(FLAGS) functions.c

clean:
	rm -f $(OBJS) $(OUT)
