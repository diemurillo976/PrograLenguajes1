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
int getPortNumber();
int getRandom(int,int);
int verifyMessage(char *);

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int portno;
    char finalUser[50];
    struct hostent *server;
    int random;

    if (argc < 2) {
       fprintf(stderr,"usage: %s hostname\n", argv[0]);
       exit(0);
    }
    
    if(argc == 3){
        strcpy(finalUser, "1");
        strcat(finalUser, argv[2]);
    }else
        strcpy(finalUser, "Nel");
    
    portno = getPortNumber();
    server = gethostbyname(argv[1]);
	
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
	
    int socialSocket = socket(AF_INET, SOCK_DGRAM, 0);
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
    
    sendto(socialSocket, (const char *)finalUser, strlen(finalUser), 
        	MSG_CONFIRM, (const struct sockaddr *) &servUDP,  
            sizeof(servUDP)); 
	
    printf("Connected!.\n"); 
  	standByYou(&servUDPGeneral, socialSocket);

    return 0;
}

void standByYou(struct sockaddr_in* serv_addr, int sockUDP){
	short breakUpFlag = 0;
	char buffer[256];
	int len, n, pid, thirdSonId, rightMessageFormat;

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
			n = recvfrom(sockUDP, (char *)buffer, 256, MSG_WAITALL, (struct sockaddr *) serv_addr, &len); 
			buffer[n] = '\0'; 
			printf(">%s", buffer); 
		}
		//original process that will write
		else{
			bzero(buffer,256);
			fgets(buffer,255,stdin);
            rightMessageFormat = verifyMessage(buffer);
            if(rightMessageFormat){
                if(isCommand(buffer)){
                    if (buffer[5] == 'e'){
                        kill(thirdSonId,SIGKILL);
                        breakUpFlag = 1;	
                    }
                }
                int n = sendto(sockUDP,(const char *) buffer, 255, MSG_CONFIRM, (const struct sockaddr *) serv_addr, sizeof(*serv_addr));
                if (n < 0) 
                    error("ERROR writing to socket");
            }else{
                printf("ERROR, wrong message format.\n");
                printf("Usage => username: message\n");   
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
	fclose(configFile);
	
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
	return port;
}

int getRandom(int min, int max){
   return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

int verifyMessage(char *input){
	char *format = strchr(input,':');
    if(isCommand(input))
        return 1;
    else{
        if(format == NULL)
            return 0;
        else
            return 1;
    }
}