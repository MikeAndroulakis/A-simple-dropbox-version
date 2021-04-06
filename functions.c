#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/wait.h>
#include "functions.h"
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#include <dirent.h>
#define SA struct sockaddr //gia th conect
#define SIZE 100000

int storedclientip;//metablhth poy apothikeyetai to clientip gia syndesh me ton server (me signal) se periptwsh poy termatisei
int storedclientport;//metablhth poy apothikeyetai to clientport gia syndesh me ton server (me signal) se periptwsh poy termatisei
char storedserverip[30];//metablhth poy apothikeyetai to serverip gia syndesh me ton server (me signal) se periptwsh poy termatisei
int storedserverport;//metablhth poy apothikeyetai to serverport gia syndesh me ton server (me signal) se periptwsh poy termatisei
int checker;//metablhth poy elegxei an termatise to programma

int sock;       /* The socket file descriptor for our "listening"socket */
int connectlist[5];  /* Array of connected sockets so we know who we are talking to */
fd_set socks;        /* Socket file descriptors we want to wakeup for, using select() */
int highsock;


void checkHostName(int hostname) 
{
    if (hostname == -1)
    {
        perror("gethostname"); 
        exit(1); 
    }
}

// Returns host information corresponding to host name 
void checkHostEntry(struct hostent * hostentry) 
{
    if (hostentry == NULL) 
    {
        perror("gethostbyname"); 
        exit(1); 
    }
}

// Converts space-delimited IPv4 addresses 
// to dotted-decimal format 
void checkIPbuffer(char *IPbuffer) 
{
    if (NULL == IPbuffer) 
    {
        perror("inet_ntoa"); 
        exit(1); 
    }
}

int getintegerclientportfrombuf(char *buf){//metatrepei to string port se integer
	int i=0;
	int intclientport;
	while (i<SIZE){
		if(buf[i]==','){
			i++;
			int j=i;
			int count=0;
			while(buf[i]!='>'){
				i++;
				count++;
			}
			char *port=malloc((count+1)*sizeof(char));
			bzero(port, count+1);
			i=j;//ksanarxizoyme to diavasma toy port
			j=0;
			while(buf[i]!='>'){
				port[j]=buf[i];
				i++;
				j++;
			}
			port[j]='\0';
			intclientport=strtol(port,NULL,10);
			break;
		}
		i++;
	}
	intclientport=ntohl(intclientport);
	return intclientport;
}

int getthenumberofelementsoftheiparray(char *buf){//briskei poses theseis exei to ip gia dhmioyrgia katallhloy pinaka
	int i=0;
	int count=0;
	while(i<SIZE){
		if(buf[i]=='<'){
			i++;
			while(buf[i]!=','){
				count++;
				i++;
			}
			break;
		}
		i++;
	}
	return count;
}

char *createthestringclientip(int count,char *buf){//edw apothikeyetai to string se enan pinaka kai epistrefetai
	int i=0;
	char *clientip=malloc((count+1)*sizeof(char));
	bzero(clientip, count+1);
	while(i<SIZE){
		if(buf[i]=='<'){
			i++;
			int j=0;
			while(buf[i]!=','){
				clientip[j]=buf[i];
				i++;
				j++;
			}
			clientip[j]='\0';
			//printf("The client IP is %s\n",clientip);
			break;
		}
		i++;
	}
	return clientip;
}

client *createnewclient(int clientip,int clientport){//dhmioyrgia kainoyrioy client gia th lista
	client *newclient=malloc(sizeof(client));
	newclient->clientip=clientip;
	newclient->clientport=clientport;
	newclient->nextclient=NULL;
	return newclient;
}

int checkifthisclientalreadyexist(client *firstclient,int clientip,int clientport){//elegxos an aytos o client hdh yparxei sth lista
	client *nextcl;
	nextcl=firstclient;
	while(nextcl!=NULL){
		if(nextcl->clientport==clientport){//already exist
			if(nextcl->clientip==clientip){
				return 1;
			}
		}
		nextcl=nextcl->nextclient;
	}
	return 0;
}

void sig_handler(int signo){//ayth h synarthsh einai gia termatismo kai diagrfh arxeiwn
    if((signo==SIGQUIT)||(signo==SIGINT)){//an labame aithma termatismoy
    	char *buf=malloc(SIZE*sizeof(char));
    	strcat(buf,"LOG_OFF <");
    	char ip[15];
		sprintf(ip,"%d",storedclientip);
    	strcat(buf,ip);
    	strcat(buf,",");
    	char port[12];
		sprintf(port,"%d",htonl(storedclientport));
		strcat(buf,port);
    	strcat(buf,">");
    	//printf("quiting buf=%s\n",buf);
    	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    	if (sockfd < 0)
			perror("socket");
		struct sockaddr_in serv_addr;
		bzero(&serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(storedserverip);
		serv_addr.sin_port = htons(storedserverport);
		if (connect(sockfd, (SA*)&serv_addr, sizeof(serv_addr)) < 0)
			perror("error connecting");
		int n = write(sockfd,buf,strlen(buf));//stelnei log off mynhma ston server
		if (n < 0)
			perror("ERROR writing to socket");
		checker=1;//h extern metablhth ginetai 1 wste na termatisei to programma
		close(sockfd);
	}
}

circ_buf *circ_buf_define(int numofelements){//dhmioyrgia toy kyklikoy buffer
    circ_buf *my_circ_buf=malloc(sizeof(circ_buf));
    
    int i;
    
    my_circ_buf->buffer=malloc(numofelements*sizeof(element *));
    for(i=0;i<numofelements;i++){
    	my_circ_buf->buffer[i]=malloc(sizeof(element));
    	my_circ_buf->buffer[i]->pathname=malloc(200*sizeof(char));
	}
    
    my_circ_buf->numofelements=numofelements;
    my_circ_buf->head=0;
    my_circ_buf->tail=0;
    return my_circ_buf;
}



int circ_buf_push(circ_buf *c,char *pathname,int version,int ip,int port){//prosthesi stoixeioy ston kykliko buffer
    int next;
    next = c->head + 1;  // next is where head will point to after this write.
    if (next >= c->numofelements){
        next = 0;
    }
    if (next == c->tail)  // if the head + 1 == tail, circular buffer is full
        return -1;

	
	c->buffer[c->head]->pathname=pathname;
	c->buffer[c->head]->version=version;
	c->buffer[c->head]->ip=ip;
	c->buffer[c->head]->port=port;
    //c->buffer[c->head] = data;  // Load data and then move
    c->head = next;             // head to next data offset.
    return 0;  // return success to indicate successful push.
}

int circ_buf_pop(circ_buf *c){//afairesi stoixeioy apo ton kykliko buffer
    int next;
    if (c->head == c->tail){  // if the head == tail, we don't have any data
        return -1;
	}
	
    next = c->tail + 1;  // next is where tail will point to after this read.
    if(next >= c->numofelements){
        next = 0;
    }
    //*data = c->buffer[c->tail];  // Read data and then move
    c->tail = next;              // tail to next offset.
    return 0;  // return success to indicate successful push.
}


char *getfilelist(char *directory){//epistrefetai mia lista me ola ta arxeia toy directory
	char *buf=malloc(SIZE*sizeof(char));
	bzero(buf,SIZE);
	struct dirent *de;
	DIR *dir=opendir(directory);
	char *version=malloc(2*sizeof(char));
	while((de=readdir(dir))!=NULL){//gia kathe arxeio sto common(diladi gia kathe client)
        if(strcmp(de->d_name,".")==0||strcmp(de->d_name,"..")==0){//kryfa arxeia de mas endiaferoyn
            continue;
	    }
	    else{
	    	if(de->d_type==DT_DIR){//an brethike kai allo directory
	    		char *tmpdir=malloc(SIZE*sizeof(char));
	    		strcpy(tmpdir,directory);
	    		strcat(tmpdir,"/");
	    		strcat(tmpdir,de->d_name);
	    		DIR *checkdir=opendir(tmpdir);
	    		struct dirent *d;
	    		int countoffilesoftmpdir=0;
	    		while((d=readdir(checkdir))!=NULL){
	    			countoffilesoftmpdir++;
				}
				closedir(checkdir);
				if(countoffilesoftmpdir<=2){//elegxoyme an den einai adeio
					continue;
				}
	    		else{//an den einai adeio
		    		strcat(buf,getfilelist(tmpdir));//ginetai anadromh
		    		continue;
		    	}
			}
			else{
		        strcat(buf,"<");
		        strcat(buf,directory);
		        strcat(buf,"/");
		        strcat(buf,de->d_name);
		        strcat(buf,",");
		        sprintf(version,"%d",0);
		        strcat(buf,version);
		        strcat(buf,">");
		    }
		}       		
	}
	closedir(dir);
	return buf;
}

int getnumberoffilesofdir(char *directory){//epistrofh toy arithmoy twn arxeiwn enos directory
	int numoffiles=0;
	struct dirent *de;
	DIR *dir=opendir(directory);
	while((de=readdir(dir))!=NULL){//gia kathe arxeio sto common(diladi gia kathe client)
	    if(strcmp(de->d_name,".")==0||strcmp(de->d_name,"..")==0){//kryfa arxeia de mas endiaferoyn
	        continue;
		}
		else{
			if(de->d_type==DT_DIR){//ama brethike kai allo directory
		    	char *tmpdir=malloc(SIZE*sizeof(char));
		    	strcpy(tmpdir,directory);
		   		strcat(tmpdir,"/");
		   		strcat(tmpdir,de->d_name);
		   		DIR *checkdir=opendir(tmpdir);
		   		struct dirent *d;
	    		int countoffilesoftmpdir=0;
		    	while((d=readdir(checkdir))!=NULL){
		   			countoffilesoftmpdir++;
				}
				closedir(checkdir);
				if(countoffilesoftmpdir<=2){//an einai adeio
					continue;
				}
		    	else{//an den einai adeio tote me anadromh ksanakaloyme th synarthsh
		    		numoffiles+=getnumberoffilesofdir(tmpdir);
		    		continue;
		    	}
			}
			else{
				numoffiles++;
			}
		}
	}
	closedir(dir);
	return numoffiles;
}


void setnonblocking(int sock){//synarthsh gia orismos non blocking sta sockets
    int opts;
    opts = fcntl(sock, F_GETFL);
    if (opts < 0) {
        perror("fcntl(F_GETFL)");
        exit(EXIT_FAILURE);
    }
    opts = (opts | O_NONBLOCK);
    if (fcntl(sock, F_SETFL, opts) < 0) {
        perror("fcntl(F_SETFL)");
        exit(EXIT_FAILURE);
    }
}

int getnumoffiles(char *buf){//pairnei to string kai epistrefei ton arithmo twn files
	int i=0;
	int numoffiles=0;
	while(i<SIZE){
		if(buf[i]==' '){
			i++;
			char *num=malloc(6*sizeof(char));
			int j=0;
			while(buf[i]!='<'){
				
				num[j]=buf[i];
				j++;
				i++;
			}
			num[j]='\0';
			//printf("AAAAAAAAAAAAAAAAAAA %s\n",num);
			numoffiles=atoi(num);
			break;
		}
		i++;
	}
	//printf("NUMOFFILESSSS %d\n",numoffiles);
	return numoffiles;
}

