#include "structures.h"

extern int storedclientip;//global
extern int storedclientport;//global
extern char storedserverip[30];
extern int storedserverport;
extern int checker;
extern int flagmessagequeue;

extern int sock;       /* The socket file descriptor for our "listening"socket */
extern int connectlist[5];  /* Array of connected sockets so we know who we are talking to */
extern fd_set socks;        /* Socket file descriptors we want to wakeup for, using select() */
extern int highsock;    /* Highest #'d file desc, needed for select() */


void checkHostName(int hostname);

void checkHostEntry(struct hostent * hostentry);

void checkIPbuffer(char *IPbuffer);

void sig_handler(int signo);

int getintegerclientportfrombuf(char *buf);

client *createnewclient(int clientip,int clientport);

int checkifthisclientalreadyexist(client *firstclient,int clientip,int clientport);

int getthenumberofelementsoftheiparray(char *buf);

char *createthestringclientip(int count,char *buf);

void sig_handlerserver(int signo);

circ_buf *circ_buf_define(int numofelements);

int circ_buf_push(circ_buf *c,char *pathname,int version,int ip,int port);

int circ_buf_pop(circ_buf *c);

char *getfilelist(char *directory);

int getnumberoffilesofdir(char *directory);

void setnonblocking(int sock);

int getnumoffiles(char *buf);
