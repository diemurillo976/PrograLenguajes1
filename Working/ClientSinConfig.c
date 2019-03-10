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

void standByYou(struct sockaddr_in*, int);

short isCommand(char*);

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int portno;
    struct hostent *server;

    
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    

    int socialSocket = socket(AF_INET, SOCK_DGRAM, 0);
	char *hello = "Hel";
	struct sockaddr_in     servUDP, servUDPGeneral; 

	bzero((char *) &servUDP, sizeof(servUDP));
	servUDP.sin_family = AF_INET; 
    servUDP.sin_port = htons(portno); 
    bcopy((char *)server->h_addr, 
         (char *)&servUDP.sin_addr.s_addr,
         server->h_length);

	bzero((char *) &servUDPGeneral, sizeof(servUDPGeneral));
	servUDPGeneral.sin_family = AF_INET; 
    servUDPGeneral.sin_port = htons(portno + 1); 
    bcopy((char *)server->h_addr, 
         (char *)&servUDPGeneral.sin_addr.s_addr,
         server->h_length);
	
	int n, len; 
    
    sendto(socialSocket, (const char *)hello, strlen(hello), 
        MSG_CONFIRM, (const struct sockaddr *) &servUDP,  
            sizeof(servUDP)); 
	
    printf("UDP message sent.\n"); 
          
  	standByYou(&servUDPGeneral, socialSocket);


    return 0;
}

void standByYou(struct sockaddr_in* serv_addr, int sockUDP){
	short breakUpFlag = 0;
	char buffer[256];
	int len, n;
	int pid;
	int thirdSonId;

	pid = fork();

	if(pid < 0){
		error("ERROR on fork");
		return;
	}
	
	if (pid == 0){
		thirdSonId = getpid();
		
	}

	while(!breakUpFlag){
	
		//forked process that will read
		if (pid == 0){
			bzero(buffer,256);
			n = recvfrom(sockUDP, (char *)buffer, 256,  
			            MSG_WAITALL, (struct sockaddr *) serv_addr, 
			            &len); 
			buffer[n] = '\0'; 
			printf("Server %d : %s\n", n, buffer); 

		}
		//original process that will write
		else{
			printf("Please enter the message: ");
			bzero(buffer,256);
			fgets(buffer,255,stdin);
			int n = sendto(sockUDP,(const char *) buffer, 255,  
					   MSG_CONFIRM, (const struct sockaddr *) serv_addr, 
						  sizeof(*serv_addr));
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



//checks for command input. Format: "comm _command symbol_"
short isCommand(char* input){
	
	int s = strlen(input);
	
	if (s >= 7 &&*(input) == 'c' && *(++input) == 'o' && *(++input) == 'm' && *(++input) == 'm' && *(++input) == ' '){
		return 1;
	}
	return 0;
}
