#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <pthread.h>

char* words[100] ={"abecedarian","abracadabra","accoutrements","adagio","aficionado","agita","agog","akimbo","alfresco","aloof","ambrosial","amok","ampersand","anemone","anthropomorphic","antimacassar","aplomb","apogee","apoplectic","appaloosa","apparatus","archipelago","atingle","avuncular","ayuwoki","babushka","bailiwick","bafflegab","balderdash","ballistic","bamboozle","bandwagon","barnstorming","beanpole","bedlam","befuddled","bellwether","berserk","bibliopole","bigmouth","biliyin","blabbermouth","blatherskite","blindside","blob","blockhead","blowback","blowhard","blubbering","bluestockings","boing","boffo (boffola)","bombastic","bonanza","bonkers","boondocks","boondoggle","borborygmus","bozo","braggadocio","brainstorm","brannigan","breakneck","brouhaha","buckaroo","bucolic","buffoon","bugaboo","bugbear","bulbous","bumbledom","bumfuzzle","bumpkin","bungalow","bunkum","bupkis","uyaje","busybody","cacophony","cahoots","calamity","calliope","candelabra","canoodle","cantankerous","catamaran","catastrophe","catawampus","caterwaul","heehee","chichi","chimerical","chimichanga","chitchat","claptrap","clishmaclaver","clodhopper","cockamamie","cockatoo","codswallop"};
char digits[10] = {'0','1','2','3','4','5','6','7','8','9'};

int getRandom(int,int);
char* generateId(char*);
int getPortNumber();
short verifyUserAssignation(char *user);
char *getUser(char *, char *);
void getUserIdFromMessage(char *, char *);
void getMessage(char *, char *);
void getIPAddress(char *);



struct infoCard{
	char userId[40];
	
};

struct user{
	struct infoCard info;
	short online;
	struct sockaddr_in addr;
};

struct user* users = NULL;

void addOnlineUser(struct infoCard* info, struct sockaddr_in* address){
	int i;
	for (i = 0; i < 100; i++){
		if((*(users+i)).online == 0){
			strcpy((*(users+i)).info.userId, (*info).userId);
			
			//memcpy(&((*(users+i)).addr), &address,
    				//sizeof(struct sockaddr_in));
			(*(users+i)).addr = *address;
			(*(users+i)).online = 1;
			return;
		}
	}
}

struct user* findOnlineUser(char* userId){
	int i;
	for (i = 0; i < 100; i++){
		if((*(users+i)).online == 1 && strcmp((*(users+i)).info.userId, userId) == 0){
			
			return users+i;
		}
	}
}
char *findUserByAddress(struct sockaddr_in* address){
	int i;
	int lenUser=20;
	char bufferUser[lenUser];
	int lenTemp=20;
	char bufferTemp[lenTemp];

	bzero(bufferUser, lenUser);
	inet_ntop(AF_INET, &((*address).sin_addr), bufferUser, lenUser);
 
	for (i = 0; i < 100; i++){
		
		if((*(users+i)).online == 1){

			bzero(bufferTemp, lenTemp);
			inet_ntop(AF_INET, &((*(users+i)).addr.sin_addr), bufferTemp, lenTemp);	

			if(strcmp(bufferUser, bufferTemp) == 0 && ((*(users+i)).addr.sin_port == (*address).sin_port)){
				return (*(users+i)).info.userId;
			}	
		}		
	
	}
}

void removeOnlineUser(char* userId){
	struct user* downUser = findOnlineUser(userId);
	
	(*(downUser)).online = 0;
}

void printOnlineUsers(){
	int i;
	int len=20;
	char buffer[len];
	

	for (i = 0; i < 100; i++){
		if((*(users+i)).online == 1){
			printf("user %s \n",(*(users+i)).info.userId);
			inet_ntop(AF_INET, &((*(users+i)).addr.sin_addr), buffer, len);
			printf("address: %s\n", buffer);
			int puerto = ntohs((*(users+i)).addr.sin_port);
			printf("port: %d\n", puerto);
			printf("-------------------------------------\n");
		}
	}
}

void standByMe(int);
short isCommand(char*);
void error(const char *msg)
{
	perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    signal(SIGCHLD,SIG_IGN);//prevents zombie processes
    srand( (unsigned) time(NULL) * getpid());//reseeds the randomgenerator
    short assignUser;
	char *userSend;
	
    //reserves shared memory for users array and initializes the array
    users = mmap(NULL, sizeof(struct user)*100, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	int i;
	for (i = 0; i < 100; i++){
		(*(users+i)).online = 0;	
		bzero((char *) &((*(users+i)).addr), sizeof((*(users+i)).addr));
	}
	
	int socketUDP;
	int socketGeneral;
    int portNumber; //Port number on which the server accepts connections
    int pid;
	int random;
	char ipAddress[15];
	bzero(ipAddress, 15);
    socklen_t clientAddressLength; //size of the address of the client
	struct sockaddr_in servUDP_addr, servUDPGeneral_addr, cli_addr;//Direccion del servidor y del cliente
   
    int n;	//return value for the read() and write() calls
	
	if (argc > 1){
    	fprintf(stderr,"ERROR, port provided\n");
        exit(1);
    }
	
	socketUDP = socket(AF_INET, SOCK_DGRAM, 0); //Peticiones de conectarse
    if (socketUDP < 0) 
       error("ERROR opening UDP server socket");
	socketGeneral = socket(AF_INET, SOCK_DGRAM, 0); //Escuchar cada cliente
    if (socketGeneral < 0) 
       error("ERROR opening general UDP server socket");

	bzero((char *) &servUDP_addr, sizeof(servUDP_addr));
	bzero((char *) &servUDPGeneral_addr, sizeof(servUDPGeneral_addr));
    portNumber = getPortNumber(); //gets the port number from the config file
	printf("Port:%d\n",portNumber);
	
	getIPAddress(ipAddress);
	
	//Setting parametes of the struct sockaddr_in
	servUDP_addr.sin_family = AF_INET;
    servUDP_addr.sin_addr.s_addr = inet_addr(ipAddress);
    servUDP_addr.sin_port = htons(portNumber); 

	servUDPGeneral_addr.sin_family = AF_INET;
    servUDPGeneral_addr.sin_addr.s_addr = inet_addr(ipAddress);
    servUDPGeneral_addr.sin_port = htons(portNumber + 1);

	//bind() = binds a socket to an address, assings a name to an unnamed socket. Return 1 on success
	//The address of the current host and port number on which the server will run
	//3 arguments: socket file descriptor, address to which is bound, size of the address to which is bound
	if (bind(socketUDP, (struct sockaddr *) &servUDP_addr, sizeof(servUDP_addr)) < 0) 
    	error("ERROR on binding UDP server");
	 
	if (bind(socketGeneral, (struct sockaddr *) &servUDPGeneral_addr, sizeof(servUDPGeneral_addr)) < 0) 
        error("ERROR on binding general UDP server");
	
    clientAddressLength = sizeof(cli_addr);
	
	//To get the host name
	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);
	printf("Hostname: %s\n", hostname);
	int lenLocalAddr=20;
	char bufferLocalAddr[lenLocalAddr];
	inet_ntop(AF_INET, &(servUDP_addr.sin_addr), bufferLocalAddr, lenLocalAddr);
	printf("address: %s\n", bufferLocalAddr);

	
			
			
	
	while(1)
	{
		char tempBuf[50]; 
		bzero((char *) tempBuf,sizeof(tempBuf)); //tempBuf needs to be clean because of possible past users
		socklen_t lenUDP; 
		lenUDP = sizeof(cli_addr);
		recvfrom(socketUDP, (char *)tempBuf, 50,  
		            MSG_WAITALL, ( struct sockaddr *) &cli_addr, 
		            &lenUDP); 
		assignUser = verifyUserAssignation(tempBuf);
		struct infoCard myInfo;
		char *myId = malloc( (size_t)50 );
		if (assignUser){
			generateId(myId);
			strcpy(myInfo.userId,myId);
		}else{
			char myUser[50];
			bzero((char *) myUser,sizeof(myUser));
			myId = getUser(tempBuf, myUser);
			strcpy(myInfo.userId,myId);
		}
		
		printf("---------------New client connected---------------\n"); 
		int lene=20;
		char buffere[lene];
		inet_ntop(AF_INET, &(cli_addr.sin_addr), buffere, lene);
		printf("Address:%s\n",buffere);

		addOnlineUser(&myInfo, &cli_addr);
		printf("Welcome: %s\n--------------------------------------------\n", myId);

		pid = fork();
		if(pid < 0)
			error("ERROR on fork");
		if(pid == 0){
			standByMe( socketGeneral);
			exit(0);
		}
	}
	return 0;
}

void standByMe( int sockUDP){
	struct sockaddr_in cli_addr;
	unsigned int n;
	n = sizeof(cli_addr);
	char buffer[256];
	int success;
	
	short breakUpFlag = 0;
	char *userMessage;
	
	while (!breakUpFlag){
		bzero(buffer,256);
		recvfrom(sockUDP, (char *)buffer, 256,  
		            MSG_WAITALL, ( struct sockaddr *) &cli_addr, 
		            &n); 
		
		userMessage = findUserByAddress(&cli_addr);		
		if (n < 0)
			error("ERROR reading from socket");
		
		if (isCommand(buffer)){
			char command = buffer[5];
			
			switch (command){
				case 'e':
					breakUpFlag = 1;
					removeOnlineUser(userMessage);
					success = sendto(sockUDP,(const char *) '$', 1,  
					   MSG_CONFIRM, (const struct sockaddr *) &cli_addr, 
						  sizeof(cli_addr));
					
					break;
				case 'g':
					printf("Printing new id...\n");
					char ss[40] = "";
					generateId(ss);
					strcat(ss,"\n");
					success = sendto(sockUDP,(const char *) ss, 40,  
					   MSG_CONFIRM, (const struct sockaddr *) &cli_addr, 
						  sizeof(cli_addr));
					
					break;

				case 'p':
					printf("\n\nPrinting online users... \n");
					printOnlineUsers();
					
					success = sendto(sockUDP, "Printing list on server...\n", 30,  
					   MSG_CONFIRM, (const struct sockaddr *) &cli_addr, 
						  sizeof(cli_addr));
						  
					break;
				default:
					printf("Invalid command!: %c \n", command);
					
					success = sendto(sockUDP, "Invalid command!\n", 22,  
					   MSG_CONFIRM, (const struct sockaddr *) &cli_addr, 
						  sizeof(cli_addr));
			}
		}
		else{
			char userId[20];
			char message[256];
			char finalMessage[256];
			bzero(userId,20);
			bzero(message,256);
			bzero(finalMessage,256);
			getUserIdFromMessage(buffer,userId);
			getMessage(buffer,message);
			strcpy(finalMessage,userMessage);
			strcat(finalMessage,":");
			strcat(finalMessage,message);
			struct user *usuario = findOnlineUser(userId);
			success = sendto(sockUDP, (const char *)finalMessage,256,
				  MSG_CONFIRM, (const struct sockaddr *) &((*usuario).addr),
				  sizeof((*usuario).addr));
			if (success == -1)
				printf("error sending message\n");
			else
				printf("message sent\n");
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

int getRandom(int min, int max){
   return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

char* generateId(char* dest){
	char* randomWord = words[getRandom(0,99)];
	char randomDigits[3] = {digits[getRandom(0,9)],digits[getRandom(0,9)],'\0'};
	strcat(dest, randomWord);
	strcat(dest, randomDigits);
	return dest;
}

void getUserIdFromMessage(char *input, char *userId){
	int i = 0;
	char *c = input;
	char *format = strchr(input,':');
	
	if(format == NULL){
		printf("ERROR, wrong message format.\n");
		error("Usage => username: message\n");
		return;
	}
	else{
		while(*(c+i) != *format){
			userId[i] = *(c+i);
			i++;
		}
		userId[i] = '\0';
		return;
	}
}

void getMessage(char *input, char *message){
	int i = 0;
	char *format = strchr(input,':');
	if(format == NULL){
		printf("ERROR, wrong message format.\n");
		error("Usage => username: message\n");
		return;
	}
	else{
		format++;
		while(*(format+i) != '\0'){
			message[i] = *(format+i);
			i++;
		}
		message[i] = '\0';
		return;
	}
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
	
	char ct[100];
	bzero((char *)&ct[0], sizeof(ct));
	fscanf(configFile,"%s",(char *)&ct[0]);
	char *c = &ct[0];
	fclose(configFile);
	
	while(*(c+i) == 'p' || *(c+i) == 'o' || *(c+i) == 'r' || *(c+i) == 't' || *(c+i) == 'N' ||
		  *(c+i) == 'u' || *(c+i) == 'm' || *(c+i) == 'b' || *(c+i) == 'e' || *(c+i) == 'r' ||
		  *(c+i) == '=')
		i++;
		
	char portN[7];
	
	while(c[i] != ';'){
		portN[e] = c[i];
		i++;
		e++;
	}
	
	port = atoi(&portN[0]);
	return port;
}

void getIPAddress(char *ip){
	bzero(ip,sizeof(ip));
	int e = 0;
	FILE *configFile = fopen("portNumber.ini","r");
	
	if(configFile == NULL){
		printf("ERROR opening file\n");
		exit(-1);
	}
	
	char ct[200];
	bzero((char *)&ct[0], sizeof(ct));
	fscanf(configFile,"%s",(char *)&ct[0]);
	char *c = strchr(ct,'?');
	fclose(configFile);
	c++;
	
	while(*c == 'i' || *c == 'p' || *c == 'A' || *c == 'd' || *c == 'r' || *c == 'e' || *c == 's' ||
		  *c == '=')
		c++;
	
	while(*c != ';'){
		ip[e] = *c;
		e++;
		c++;
	}
}

short verifyUserAssignation(char *user){
	short i = 1;
	if (user[0] == '1')
		i = 0;
	return i;
}

char* getUser(char *user, char *myUser){
	char c = user[1];
	int i = 1;
	int e = 0;
	while (user[i] != '\0'){
		myUser[e] = user[i];
		e++;
		i++;
	}
	myUser[e] = user[i];
	return myUser;
}
