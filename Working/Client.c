#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <signal.h>
#include <arpa/inet.h>

void standByYou(int, struct sockaddr_in*, int);
void standByThem(int, struct sockaddr_in*);
short isCommand(char*);
int getPortNumber();

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{

    int sockfd, portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    
    if (argc < 2) {
       fprintf(stderr,"usage %s hostname\n", argv[0]);
       exit(0);
    }
    portno = getPortNumber();
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");


    int socialSocket = socket(AF_INET, SOCK_DGRAM, 0);
	char *hello = "Hel";
	struct sockaddr_in     servUDP; 

	servUDP.sin_family = AF_INET; 
    servUDP.sin_port = htons(portno + 1); 
    bcopy((char *)server->h_addr, 
         (char *)&servUDP.sin_addr.s_addr,
         server->h_length);

	
	int n, len; 
    
    sendto(socialSocket, (const char *)hello, strlen(hello), 
        MSG_CONFIRM, (const struct sockaddr *) &servUDP,  
            sizeof(servUDP)); 
    printf("UDP message sent.\n"); 
          
  	standByYou(sockfd, &servUDP, socialSocket);

    close(sockfd);
    return 0;
}

void standByYou(int sockfd, struct sockaddr_in* serv_addr, int sockUDP){
	short breakUpFlag = 0;
	char buffer[256];
	int n;
	int pid;
	int thirdSonId;

	pid = fork();
	if (pid == 0){
		thirdSonId = getpid();
		standByThem(sockUDP, serv_addr);
	}

	

	pid = fork();

	if(pid < 0){
		error("ERROR on fork");
		return;
	}

	while(!breakUpFlag){
	
		//forked process that will read
		if (pid == 0){
			

		}
		//original process that will write
		else{
			//printf("Please enter the message: ");
			bzero(buffer,256);
			fgets(buffer,255,stdin);
			n = write(sockfd,buffer,strlen(buffer));
			if (n < 0) 
				error("ERROR writing to socket");
			if(isCommand(buffer)){
				if (buffer[5] == 'e'){
					kill(thirdSonId,SIGKILL);
					breakUpFlag = 1;	
				}		
			}
		}
		
	}
}

void standByThem(int socialSocket, struct sockaddr_in* servaddr){
	
	char buffer[256]; 
	int n, len; 
      
    while(1){
		printf("standbythem\n");
		bzero(buffer,256);
		n = recvfrom(socialSocket, (char *)buffer, 9,  
		            MSG_WAITALL, (struct sockaddr *) &servaddr, 
		            &len); 
		buffer[n] = '\0'; 
		printf("Server %d : %s\n", n, buffer); 

	}

	
}


//checks for command input. Format: "comm _command symbol_"
short isCommand(char* input){
	
	int s = strlen(input);
	
	if (s >= 7 &&*(input) == 'c' && *(++input) == 'o' && *(++input) == 'm' && *(++input) == 'm' && *(++input) == ' '){
		return 1;
	}
	return 0;
}

//returns the port number from a file

int getPortNumber(){
	
	int port = 0;
	int i = 0;
	int e = 0;
	
	FILE *configFile = fopen("portNumber.ini","r");
	
	if(configFile == NULL){
		printf("ERROR opening file\n");
		exit(-1);
	}
	
	char ct[50];
	bzero((char *)&ct[0], sizeof(ct));
	fscanf(configFile,"%s",(char *)&ct[0]);
	
	char *c = &ct[0];
	
	while(*(c+i) == 'p' || *(c+i) == 'o' || *(c+i) == 'r' || *(c+i) == 't' || *(c+i) == 'N' ||
		  *(c+i) == 'u' || *(c+i) == 'm' || *(c+i) == 'b' || *(c+i) == 'e' || *(c+i) == 'r' ||
		  *(c+i) == '=')
		i++;
		
	char portN[7];
	
	while(c[i] != '\0'){
		portN[e] = c[i];
		i++;
		e++;
	}
	
	port = atoi(&portN[0]);
		
	fclose(configFile);
	
	return port;
}









