#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <unistd.h> 
#include <netdb.h> 
#include <stdlib.h> 
#include <string.h>
#include "functions.h"
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <dirent.h>
#define SA struct sockaddr //gia th conect
#define perror2(s,e) fprintf( stderr , " %s : %s \n",s,strerror(e))
#define SIZE 100000



int main(int argc,char *argv[]){
	fd_set readfds;
    checker=0;//EXTERN
    char hostbuffer[256];
    char *clientIP=malloc(20*sizeof(char));//to ip toy client se string
    
    struct hostent *host_entry;
    gethostname(hostbuffer,sizeof(hostbuffer));
    host_entry = gethostbyname(hostbuffer);
    checkHostEntry(host_entry);
    clientIP=inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
    int clientport=atoi(argv[2]);//clientport
    int serverport=atoi(argv[5]);//serverport
    printf("Host IP: %s\n", clientIP);
    printf("BinaryIP:%d\n",inet_addr(clientIP));
    
	storedclientip=inet_addr(clientIP);//EXTERN
    storedclientport=clientport;//EXTERN
    strcpy(storedserverip,argv[6]);//EXTERN
    storedserverport=serverport;//EXTERN

	struct client *firstclient=NULL;//deikths gia th lista twn clients
	struct client *previousclient=NULL;//deikths gia th lista twn clients
    struct client *nextcl=NULL;//deikths gia th lista twn clients
	int i;
    
    char *serverip=argv[6];//serverip
    struct hostent *server;
    server = gethostbyname(argv[6]);//serverip
    
    int sockfd, n;
    struct sockaddr_in serv_addr;
    char *buf=malloc(SIZE*sizeof(char));
    char *clientip;
    unsigned int clip=0;
    
    
    //int workerthreads=atoi(argv[3]);
	int buffersize=atoi(argv[4]);
	if(buffersize<sizeof(circ_buf)){
		printf("buffersize too small the client terminates\n");
		exit(0);
	}
	circ_buf *my_circ_buf=circ_buf_define(buffersize);//dhmioyrgia kyklikoy buffer
    
	pid_t mainpid;
	mainpid=fork();//fork poy to paidi perimenei aithmata enw o pateras blepei an o client termatise
	
	if(mainpid==0){//edw to paidi tha perimenei na labei aithmata
		sockfd = socket(AF_INET, SOCK_STREAM, 0);//anoigei syndesh
		if (sockfd < 0)
			perror("socket");
		if (server == NULL){
			fprintf(stderr,"ERROR, no such host");
			exit(0);
		}
		bzero(&serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(serverip);//arxikopoioyntai ta addresses gia na syndethei me to katallhlo mhxanima
		serv_addr.sin_port = htons(serverport);//arxikopoioyntai ta addresses gia na syndethei me to katallhlo mhxanima
		if (connect(sockfd, (SA*)&serv_addr, sizeof(serv_addr)) < 0)
			perror("error connecting");
		strcpy(buf,"LOG_ON <");
		char ip[12];
		sprintf(ip,"%d",inet_addr(clientIP));
		strcat(buf,ip);
		strcat(buf,",");
		char port[12];
		sprintf(port,"%d",htonl(clientport));
		strcat(buf,port);
		strcat(buf,">");
		n = write(sockfd,buf,strlen(buf));//stelnei log on mynhma ston server
		//printf("Write to server %s\n",buf);
		if (n < 0)
			perror("ERROR writing to socket");
		bzero(buf,SIZE);
		read(sockfd, buf,SIZE);//diabazei apo ton server to tuple me toys clients
		close(sockfd);
		printf("The clients connected to the server are:%s\n", buf);
		
		int numofclients=buf[12]-'0';
		int j=0;
		for(i=0;i<numofclients;i++){
			while(buf[j]!='<'){
				j++;
			}
			j++;
			int k=j;
			int count;
			while(buf[j]!=','){
				count++;
				j++;
			}
					
		    clientip=malloc((count+1)*sizeof(char));
		    j=k;
		    k=0;
		    while(buf[j]!=','){
		     	clientip[k]=buf[j];
		     	k++;
		     	j++;
			}
			clientip[k]='\0';
			j++;
			unsigned int clip;
		    clip=atoi(clientip);//integer of client ip
		    
		    count=0;
		    k=j;
		    while(buf[j]!='>'){
		     	count++;
		     	j++;
			}
			j=k;
			k=0;
			char *port=malloc((count+1)*sizeof(char));
			while(buf[j]!='>'){
				port[k]=buf[j];
				j++;
				k++;
			}
			port[k]='\0';
			int intclientport=atoi(port);
			//intclientport=ntohl(intclientport);
		    if(firstclient==NULL){
		     	firstclient=createnewclient(clip,intclientport);//prostithentai sth lista oi clients
			}
			else{
				previousclient=firstclient;
				nextcl=firstclient;
				while(nextcl!=NULL){
					previousclient=nextcl;
					nextcl=nextcl->nextclient;
				}
				previousclient->nextclient=createnewclient(clip,intclientport);//prostithentai sth lista oi clients
			}
		}
		nextcl=firstclient;
		while(nextcl!=NULL){
			printf("The client %d with port %d has been added to the buffer\n",nextcl->clientip,nextcl->clientport);
			nextcl=nextcl->nextclient;
		}
		

		nextcl=firstclient;
		while(nextcl!=NULL){//gia ton kathe client
			bzero(buf,SIZE);
			sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if (sockfd < 0)
				perror("socket");
			if (server == NULL){
				fprintf(stderr,"ERROR, no such host");
				exit(0);
			}
			bzero(&serv_addr, sizeof(serv_addr));
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_addr.s_addr = nextcl->clientip;
			serv_addr.sin_port = htons(nextcl->clientport);
			if (connect(sockfd, (SA*)&serv_addr, sizeof(serv_addr)) < 0)
				perror("error connecting");
			strcpy(buf,"GET_FILE_LIST");//stelnei get_file_list aithma
			n = write(sockfd,buf,strlen(buf));//to grafei sto socket
			printf("Requested file list from client with ip %d and port %d\n",nextcl->clientip,nextcl->clientport);
			if (n < 0)
				perror("ERROR writing to socket");
			bzero(buf,SIZE);
			read(sockfd, buf,SIZE);//diabazei to file list toy client
			printf("The file list received from client with ip %d and port %d is:\n",nextcl->clientip,nextcl->clientport);
			printf("\n%s\n",buf);
			printf("\n");
			int numoffiles=getnumoffiles(buf);//o arithmos twn arxeiwn poy apekthse
			j=0;
			for(i=0;i<numoffiles;i++){//to kathe arxeio mpainei mesa ston circular buffer
				while(buf[j]!='<'){
					j++;
				}
				j++;
				char *pathname=malloc(SIZE*sizeof(char));//metabliti gia apothikeysi toy kathe pathname(arxeioy)
				int k=0;
				while(buf[j]!=','){
					pathname[k]=buf[j];
					k++;
					j++;
				}
				pathname[k]='\0';
				j++;
				char *vers=malloc(4*sizeof(char));
				k=0;
				while(buf[j]!='>'){
					vers[k]=buf[j];
					k++;
					j++;
				}
				int version=atoi(vers);//h version toy arxeioy
				circ_buf_push(my_circ_buf,pathname,version,nextcl->clientip,nextcl->clientport);//prostithetai ston kykliko buffer
			}
			bzero(buf,SIZE);
			close(sockfd);

			
			bzero(buf,SIZE);
			sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if (sockfd < 0)
				perror("socket");
			if (server == NULL){
				fprintf(stderr,"ERROR, no such host");
				exit(0);
			}
			bzero(&serv_addr, sizeof(serv_addr));
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_addr.s_addr = nextcl->clientip;
			serv_addr.sin_port = htons(nextcl->clientport);
			if (connect(sockfd, (SA*)&serv_addr, sizeof(serv_addr)) < 0)
				perror("error connecting");
			strcpy(buf,"SEND_FILE_LIST ");
			numoffiles=getnumberoffilesofdir(argv[1]);
			char num[5];
			sprintf(num,"%d",numoffiles);
			strcat(buf,num);
			strcat(buf,getfilelist(argv[1]));
			
			n = write(sockfd,buf,strlen(buf));//stelnei to file list se opoion to zitise
			if (n < 0)
				perror("ERROR writing to socket");
			close(sockfd);
			nextcl=nextcl->nextclient;
		}
		
		
		int master_socket , new_socket , sd;
		int max_clients = 300;
		int client_socket[300];
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
	    address.sin_addr.s_addr = inet_addr(clientIP);
	    address.sin_port = htons(clientport);
	    
	    
	    int addrlen = sizeof(address);
		if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0){//bind
	        perror("bind failed");   
	        exit(EXIT_FAILURE);   
	    }
	
		if (listen(master_socket, 3) < 0){   
	        perror("listen");   
	        exit(EXIT_FAILURE);   
	    }
		int clienttmpport=0;
		while(1){//anoixtes syndeseis synexws
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
	
		    if (FD_ISSET(master_socket, &readfds)){//ama brethike syndesh
		        if ((new_socket = accept(master_socket,(struct sockaddr *)&address, (socklen_t*)&addrlen))<0){//ama ginei accept
		            perror("accept");
		            exit(EXIT_FAILURE);
		        }
		        for (i = 0; i < max_clients; i++){   
					//if position is empty  
				    if( client_socket[i] == 0 ){   
				        client_socket[i] = new_socket;   
				        break;   
				    }   
				}
				bzero(buf,SIZE);
				read(new_socket, buf, SIZE);//diabazei ayto poy toy egrapsan
				if((buf[0]=='L')&&(buf[5]=='N')){//an einai log on mynhma apo ton server tote apothikeyei ton neo client sto kykliko buffer
					clienttmpport=getintegerclientportfrombuf(buf);
					//printf("port:%d\n",clientport);
			     	int count=getthenumberofelementsoftheiparray(buf);
			     	clientip=malloc((count+1)*sizeof(char));
			     	clientip=createthestringclientip(count,buf);
			     	clip=atoi(clientip);
			     	printf("Client with ip %d and port %d has added\n",clip,clienttmpport);
			     	
			     	if(checkifthisclientalreadyexist(firstclient,clip,clienttmpport)==1){//elegxetai an yparxei hdh aytos o client
			     		printf("This client already exists\n");
			     		//strcpy(clientIP,client);
					}
					else{
				     	if(firstclient==NULL){
				     		firstclient=createnewclient(clip,clienttmpport);
						}
						else{
							previousclient=firstclient;
							nextcl=firstclient;
							while(nextcl!=NULL){
								previousclient=nextcl;
								nextcl=nextcl->nextclient;
							}
							previousclient->nextclient=createnewclient(clip,clienttmpport);
							
						}
					}
					printf("The list of the other clients are:\n");
					nextcl=firstclient;
					while(nextcl!=NULL){
						printf("%d %d\n",nextcl->clientip,nextcl->clientport);
						nextcl=nextcl->nextclient;
					}
				}
				else if((buf[0]=='L')&&(buf[5]=='F')){//an elabe log off mynhma apo ton server tote afairei ton sygkekrimeno client
					clienttmpport=getintegerclientportfrombuf(buf);
					int count=getthenumberofelementsoftheiparray(buf);
		     		clientip=malloc((count+1)*sizeof(char));
		     		clientip=createthestringclientip(count,buf);
					clip=atoi(clientip);//integer of client ip
					printf("Client with ip %d and port %d logged off\n",clip,clienttmpport);
					if(firstclient==NULL){
		     			printf("ERROR_%s_%d_NOT_FOUND_IN_LIST",clientip,clienttmpport);
					}
					else if(firstclient->nextclient==NULL){
						if((firstclient->clientip==clip)&&(firstclient->clientport==clienttmpport)){
		     				firstclient=NULL;
						}
						else{
							printf("ERROR_%d_%d_NOT_FOUND_IN_LIST",clip,clienttmpport);
						}
					}
					else if((firstclient->clientip==clip)&&(firstclient->clientport==clienttmpport)){
						firstclient=firstclient->nextclient;
					}
					else{
						nextcl=firstclient;
						previousclient=firstclient;
			     		while(nextcl!=NULL){
			     			if((nextcl->clientip==clip)&&(nextcl->clientport==clienttmpport)){
			     				previousclient->nextclient=nextcl->nextclient;
			     				break;
							}
							previousclient=nextcl;
			     			nextcl=nextcl->nextclient;
						}
						if(nextcl==NULL){
							printf("ERROR_%s_%d_NOT_FOUND_IN_LIST",clientip,clienttmpport);
						}
					}
				}
				else if((buf[0]=='G')&&(buf[9]=='L')){//an elabe get file list aithma
					bzero(buf,SIZE);
					strcpy(buf,"FILE_LIST ");
					int numoffiles=0;
					numoffiles=getnumberoffilesofdir(argv[1]);//arithmos twn arxeiwn toy directory tou
					char num[5];
					sprintf(num,"%d",numoffiles);
					strcat(buf,num);
					strcat(buf,getfilelist(argv[1]));//synarthsh poy diabazei kai apothikeyei se enan buffer ola ta pathnames twn arxeiwn
	                write(new_socket,buf,strlen(buf));
	                
				}
				else if((buf[0]=='S')&&(buf[5]=='F')){//edw stelnei aithma gia na parei thn file list enos client
					nextcl=firstclient;
					while(nextcl!=NULL){
						previousclient=nextcl;
						nextcl=nextcl->nextclient;
					}
					printf("Requested file list from client with ip %d and port %d\n",previousclient->clientip,previousclient->clientport);
					printf("The file list received from client with ip %d and port %d is:\n",previousclient->clientip,previousclient->clientport);
					printf("\n");
					for(i=5;i<SIZE;i++){
						printf("%c",buf[i]);
					}
					printf("\n");
					
					int numoffiles=getnumoffiles(buf);
					j=0;
					for(i=0;i<numoffiles;i++){
						while(buf[j]!='<'){
							j++;
						}
						j++;
						char *pathname=malloc(SIZE*sizeof(char));
						int k=0;
						while(buf[j]!=','){
							pathname[k]=buf[j];
							k++;
							j++;
						}
						pathname[k]='\0';
						j++;
						char *vers=malloc(4*sizeof(char));
						k=0;
						while(buf[j]!='>'){
							vers[k]=buf[j];
							k++;
							j++;
						}
						int version=atoi(vers);
						circ_buf_push(my_circ_buf,pathname,version,clip,clienttmpport);//bazei ena-ena ta pathnames mesa ston kykliko buffer
					}
				}
				bzero(buf,SIZE);
			}
		}
	}
	else{//parent
		while(1){//edw elegxoyme synexws an termatise o client wste na stalthei katallhlo mynhma ston server
			signal(SIGINT,sig_handler);//kaleitai synarthsh wste na eidopoihthei o server an o client termatise me ctrl+c (SIGINT)
			if(checker==1){//an stalthike mynhma ston server tote skotwnetai to paidi kai termatizoyme
				kill(mainpid,SIGKILL);//skotwnei kai th diergasia paidi
				return 0;
			}
		}
	}	
}




