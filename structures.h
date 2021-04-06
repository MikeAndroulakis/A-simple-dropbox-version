#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct client{//struct gia clients(gia client list)
    int clientip;
    int clientport;
    struct client *nextclient;
}client;

typedef struct element{//struct gia to kathe element toy kykliko buffer
	char *pathname;
	int version;
	int ip;
	int port;
}element;

typedef struct {//struct gia ton kykliko buffer
    element **buffer;
    int head;
    int tail;
    int numofelements;
}circ_buf;
