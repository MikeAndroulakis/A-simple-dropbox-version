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
#include <stdint.h>
#include <signal.h>
#include <sys/shm.h>
#define SA struct sockaddr //gia th conect
#include <pthread.h>
#define SIZE 100000


int main(int argc,char *argv[]){
	fd_set readfds;
    char hostbuffer[256];
    char *IPbuffer;
    struct hostent *host_entry;
    gethostname(hostbuffer,sizeof(hostbuffer));
    
	struct client *firstclient=NULL;//deikths gia th lista twn clients
	struct client *previousclient;//deikths gia th lista twn clients
	struct client *nextcl;//deikths gia th lista twn clients

    host_entry = gethostbyname(hostbuffer);
    checkHostEntry(host_entry);

    IPbuffer = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
    printf("Host IP: %s\n", IPbuffer);
    printf("BinaryIP:%d\n",inet_addr(IPbuffer));
    int serverport=atoi(argv[1]);
    
	int clientport=0;
	char *clientip;
	unsigned int clip=0;
	
	int master_socket , new_socket , sd;
	int i;
	int max_clients = 30;
	int client_socket[30];
    int max_sd;//maxsocket
    
    for (i = 0; i < max_clients; i++){   
        client_socket[i] = 0;   
    }
    
    
	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0){   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    } 
    int reuse_addr = 1;
	setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR,&reuse_addr, sizeof(reuse_addr));
	/* Set socket to non-blocking with our setnonblocking routine */
    setnonblocking(master_socket);
    
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = inet_addr(IPbuffer);
    address.sin_port = htons(serverport);
    
    
    int addrlen = sizeof(address);
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0){   
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    }   

	if (listen(master_socket, 3) < 0){   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }
	char *buf=malloc(SIZE*sizeof(char));
	
	while(1){//synexws perimenei syndeseis
		FD_ZERO(&readfds);
	    FD_SET(master_socket, &readfds);   
	    max_sd = master_socket;
	    for( i = 0 ; i < max_clients ; i++){   
	        sd = client_socket[i];	 
	        if(sd > 0)   
	            FD_SET( sd , &readfds);   
	        if(sd > max_sd)   
	            max_sd = sd;   
	    }
	    select( max_sd + 1 , &readfds , NULL , NULL ,NULL);

	    if (FD_ISSET(master_socket, &readfds)){//an brethike
	        if ((new_socket = accept(master_socket,(struct sockaddr *)&address, (socklen_t*)&addrlen))<0){//an ginei accept
	            perror("accept");
	            exit(EXIT_FAILURE);
	        }
	        for (i = 0; i < max_clients; i++){   
			    if( client_socket[i] == 0 ){   
			        client_socket[i] = new_socket;   
			        break;   
			    }   
			}
			bzero(buf, SIZE);
			read(new_socket, buf, SIZE);//diabazei ti toy esteilan
			
			if((buf[0]=='L')&&(buf[5]=='N')){//an einai log on mynhma
				clientport=getintegerclientportfrombuf(buf);
		     	int count=getthenumberofelementsoftheiparray(buf);
		     	clientip=malloc((count+1)*sizeof(char));
		     	clientip=createthestringclientip(count,buf);
		     	clip=atoi(clientip);//integer of client ip
	    		if(firstclient!=NULL){
		    		nextcl=firstclient;
		    		while(nextcl!=NULL){
		    			previousclient=nextcl;
		    			nextcl=nextcl->nextclient;
					}
				}
		     		    
		     	if(checkifthisclientalreadyexist(firstclient,clip,clientport)==1){//elegxei an o client yparxei hdh
		     		printf("This client already exists\n");
				}
				else{
					printf("Client with ip %d and port %d logged on to the server\n",clip,clientport);
			     	if(firstclient==NULL){
			     		firstclient=createnewclient(clip,clientport);//prostithetai o client sth lista
					}
					else{
						previousclient=firstclient;
						nextcl=firstclient;
						while(nextcl!=NULL){
							previousclient=nextcl;
							nextcl=nextcl->nextclient;
						}
						previousclient->nextclient=createnewclient(clip,clientport);//prostithetai o client sth lista
					}
					printf("Client with ip %d and port %d requested the client list\n",clip,clientport);
			    	nextcl=firstclient;
			    	bzero(buf, SIZE);
				   	strcat(buf,"CLIENT_LIST ");
				   	count=0;
				   	while(nextcl!=NULL){
						if((nextcl->clientport!=clientport)||(nextcl->clientip!=clip)){
							count++;
						}
				    	nextcl=nextcl->nextclient;
					}
					char numofclients[12];
					sprintf(numofclients,"%d",count);
					strcat(buf,numofclients);
					nextcl=firstclient;
					while(nextcl!=NULL){
			    		if((nextcl->clientport!=clientport)||(nextcl->clientip!=clip)){
							strcat(buf,"<");
							char ip[100];
							sprintf(ip,"%d",nextcl->clientip);
				   			strcat(buf,ip);
				   			strcat(buf,",");
				   			
				    		char clport[12];
				   			sprintf(clport,"%d",nextcl->clientport);
				   			strcat(buf,clport);
				   			strcat(buf,">");
				   			//printf("buf=%s\n",buf);
						}
			    		nextcl=nextcl->nextclient;
					}
				   	// and send that buffer to client 
				   	write(new_socket, buf, SIZE);//stelnei to tuple me toys syndedemenoys clients ston client poy toy esteile aithma
				}
				
				strcpy(buf,"LOG_ON <");
				char iparray[12];
				sprintf(iparray,"%d",clip);
				strcat(buf,iparray);
				strcat(buf,",");
				char port[12];
				sprintf(port,"%d",htonl(clientport));
				strcat(buf,port);
				strcat(buf,">");
				nextcl=firstclient;
				while(nextcl!=NULL){//gia olous toys ypoloipoys clients stelnetai mynhma log on oti syndethike enas client
					if((nextcl->clientip!=clip)||(nextcl->clientport!=clientport)){ //an den einai ayto poy molis prosthesame
						struct sockaddr_in serv_addr;
						int clientsock = socket(AF_INET, SOCK_STREAM, 0);
						if (clientsock < 0)
							perror("socket");
						bzero(&serv_addr, sizeof(serv_addr));
						serv_addr.sin_family = AF_INET;
						serv_addr.sin_addr.s_addr = nextcl->clientip;
						serv_addr.sin_port = htons(nextcl->clientport);

						if (connect(clientsock, (SA*)&serv_addr, sizeof(serv_addr)) < 0)
							perror("error connecting");
						write(clientsock,buf,strlen(buf));//stelnoyme mynhma log on me to tuple toy neoy client
						close(clientsock);
					}
					nextcl=nextcl->nextclient;
				}
			}
			else if((buf[0]=='L')&&(buf[5]=='F')){//an kapoios client aposyndethike
				clientport=getintegerclientportfrombuf(buf);
				int count=getthenumberofelementsoftheiparray(buf);
	     		clientip=malloc((count+1)*sizeof(char));
	     		clientip=createthestringclientip(count,buf);
				clip=atoi(clientip);//integer of client ip
				printf("Client with ip %d and port %d logged off\n",clip,clientport);
				if(firstclient==NULL){//elegxoyme an yparxei aytos o client
	     			printf("ERROR_%s_%d_NOT_FOUND_IN_LIST",clientip,clientport);//an den yparxei ektypwnoyme katallhlo error
				}
				else if(firstclient->nextclient==NULL){//elegxoyme an yparxei aytos o client
					if((firstclient->clientip==clip)&&(firstclient->clientport==clientport)){
	     				firstclient=NULL;
					}
					else{
						printf("ERROR_%d_%d_NOT_FOUND_IN_LIST",clip,clientport);
					}
				}
				else if((firstclient->clientip==clip)&&(firstclient->clientport==clientport)){
					firstclient=firstclient->nextclient;
				}
				else{
					nextcl=firstclient;
					previousclient=firstclient;
		     		while(nextcl!=NULL){
		     			if((nextcl->clientip==clip)&&(nextcl->clientport==clientport)){
		     				previousclient->nextclient=nextcl->nextclient;
		     				break;
						}
						previousclient=nextcl;
		     			nextcl=nextcl->nextclient;
					}
					if(nextcl==NULL){//elegxoyme an yparxei aytos o client
						printf("ERROR_%s_%d_NOT_FOUND_IN_LIST",clientip,clientport);
					}
				}
				bzero(buf,SIZE);
				if(firstclient!=NULL){//an yparxoyn akoma syndedemenoi clients sto systhma
					nextcl=firstclient;
					while(nextcl!=NULL){//se kathe enan client stelnete mynhma log off
						strcpy(buf,"LOG_OFF <");
				    	char ip[15];
						sprintf(ip,"%d",clip);
				    	strcat(buf,ip);
				    	strcat(buf,",");
				    	char port[12];
						sprintf(port,"%d",htonl(clientport));
						strcat(buf,port);
				    	strcat(buf,">");
						int logoffsock = socket(AF_INET, SOCK_STREAM, 0);
				    	if (logoffsock < 0)
							perror("socket");
						struct sockaddr_in serv_addr;
						bzero(&serv_addr, sizeof(serv_addr));
						serv_addr.sin_family = AF_INET;
						serv_addr.sin_addr.s_addr = nextcl->clientip;
						serv_addr.sin_port = htons(nextcl->clientport);

						if (connect(logoffsock, (SA*)&serv_addr, sizeof(serv_addr)) < 0)
							perror("error connecting");
						int n = write(logoffsock,buf,strlen(buf));
						if (n < 0)
							perror("ERROR writing to socket");
						close(logoffsock);
						nextcl=nextcl->nextclient;
					}
				}
			}
			nextcl=firstclient;
			if(nextcl==NULL){
				printf("THERE ARE NO CLIENTS INTO THE SERVER\n");
			}
			else{
				printf("THE CLIENTS CONNECTED TO THE SERVER ARE:\n");
				while(nextcl!=NULL){
					printf("CLIENT  %d %d\n",nextcl->clientip,nextcl->clientport);
					nextcl=nextcl->nextclient;
				}
			}
		}
	}
}

