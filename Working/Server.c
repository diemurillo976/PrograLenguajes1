#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h> 
#include <sys/mman.h>
#include <arpa/inet.h>

char* words[100] ={"abecedarian","abracadabra","accoutrements","adagio","aficionado","agita","agog","akimbo","alfresco","aloof","ambrosial","amok","ampersand","anemone","anthropomorphic","antimacassar","aplomb","apogee","apoplectic","appaloosa","apparatus","archipelago","atingle","avuncular","azure","babushka","bailiwick","bafflegab","balderdash","ballistic","bamboozle","bandwagon","barnstorming","beanpole","bedlam","befuddled","bellwether","berserk","bibliopole","bigmouth","bippy","blabbermouth","blatherskite","blindside","blob","blockhead","blowback","blowhard","blubbering","bluestockings","boing","boffo (boffola)","bombastic","bonanza","bonkers","boondocks","boondoggle","borborygmus","bozo","braggadocio","brainstorm","brannigan","breakneck","brouhaha","buckaroo","bucolic","buffoon","bugaboo","bugbear","bulbous","bumbledom","bumfuzzle","bumpkin","bungalow","bunkum","bupkis","burnsides","busybody","cacophony","cahoots","calamity","calliope","candelabra","canoodle","cantankerous","catamaran","catastrophe","catawampus","caterwaul","chatterbox","chichi","chimerical","chimichanga","chitchat","claptrap","clishmaclaver","clodhopper","cockamamie","cockatoo","codswallop"};
char digits[10] = {'0','1','2','3','4','5','6','7','8','9'};

int getRandom(int,int);
char* generateId(char*);
int getPortNumber();
short verifyUserAssignation(char *user);
char *getUser(char *, char *);

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

void removeOnlineUser(char* userId){
	struct user* downUser = findOnlineUser(userId);
	
	(*(downUser)).online = 0;
}

void printOnlineUsers(){
	int i;
	for (i = 0; i < 100; i++){
		if((*(users+i)).online == 1){
			printf("user %s \n",(*(users+i)).info.userId);
		}
	}
}

void standByMe(char*, int);
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
    socklen_t clientAddressLength; //size of the address of the client
	struct sockaddr_in servUDP_addr, servUDPGeneral_addr, cli_addr;//Direccion del servidor y del cliente
   
    int n;	//return value for the read() and write() calls
	
	socketUDP = socket(AF_INET, SOCK_DGRAM, 0); 
    if (socketUDP < 0) 
       error("ERROR opening UDP server socket");
	socketGeneral = socket(AF_INET, SOCK_DGRAM, 0); 
    if (socketGeneral < 0) 
       error("ERROR opening general UDP server socket");

	bzero((char *) &servUDP_addr, sizeof(servUDP_addr));
	bzero((char *) &servUDPGeneral_addr, sizeof(servUDPGeneral_addr));
    portNumber = getPortNumber(); //gets the port number from the config file
	
	//Setting parametes of the struct sockaddr_in
	servUDP_addr.sin_family = AF_INET;
    servUDP_addr.sin_addr.s_addr = INADDR_ANY;
    servUDP_addr.sin_port = htons(portNumber); //IP address of the host

	servUDPGeneral_addr.sin_family = AF_INET;
    servUDPGeneral_addr.sin_addr.s_addr = INADDR_ANY;
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

	while(1)
	{
		char tempBuf[50]; 
		bzero((char *) tempBuf,sizeof(tempBuf)); //tempBuf needs to be clean because of possible past users
		int lenUDP; 
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
		
		printf("UDP client socket connected\n"); 
		int lene=20;
		char buffere[lene];
		inet_ntop(AF_INET, &(cli_addr.sin_addr), buffere, lene);
		printf("address:%s\n",buffere);

		addOnlineUser(&myInfo, &cli_addr);

		pid = fork();
		if(pid < 0)
			error("ERROR on fork");
		if(pid == 0){
			standByMe(myId, socketGeneral);
			exit(0);
		}
	}
	return 0;
}

void standByMe(char* myId, int sockUDP){
	struct sockaddr_in cli_addr;
	unsigned int n;
	n = sizeof(cli_addr);
	char buffer[256];
	int success;
	printf("Welcome: %s  \n", myId);
	short breakUpFlag = 0;
	
	while (!breakUpFlag){
		bzero(buffer,256);
		recvfrom(sockUDP, (char *)buffer, 256,  
		            MSG_WAITALL, ( struct sockaddr *) &cli_addr, 
		            &n); 
		if (n < 0)
			error("ERROR reading from socket");
			
		buffer[n] = '\0'; 
		printf("Here is the message: %s\n",buffer);
		
		if (isCommand(buffer)){
			char command = buffer[5];
			
			switch (command){
				case 'e':
					breakUpFlag = 1;
					removeOnlineUser(myId);
					success = sendto(sockUDP,(const char *) '$', 1,  
					   MSG_CONFIRM, (const struct sockaddr *) &cli_addr, 
						  sizeof(cli_addr));
					
					break;
				case 'g':
					printf("printing new id  \n");
					char ss[35] = "";
					generateId(ss);
					success = sendto(sockUDP,(const char *) ss, strlen(ss),  
					   MSG_CONFIRM, (const struct sockaddr *) &cli_addr, 
						  sizeof(cli_addr));
					
					break;

				case 'p':
					printf("printing online users \n");
					printOnlineUsers();
					
					success = sendto(sockUDP, "printing list on server", 23,  
					   MSG_CONFIRM, (const struct sockaddr *) &cli_addr, 
						  sizeof(cli_addr));
						  
					break;

				case 's':
					printf("writing to socket %d :%d \n", buffer[7]-'0', (*(users+(buffer[7]-'0'))).addr.sin_family);
					char* hello = "que perro";
					int lene=20;
					char buffere[20];
					inet_ntop(AF_INET, &((*(users+(buffer[7]-'0'))).addr.sin_addr), buffere, lene);
					printf("address:%s %d\n",buffere,(int) strlen(hello));
					success = sendto(sockUDP, (const char *)hello, 9,  
					   MSG_CONFIRM, (const struct sockaddr *) &((*(users+(buffer[7]-'0'))).addr), 
						  sizeof((*(users+(buffer[7]-'0'))).addr));

					if (success == -1)
						printf("error sending message\n");
					else
						printf("message sent\n");
					
					//n = write(tempSocket, "que perro", 9);
					
					break;
				default:
					printf("Invalid command!: %c \n", command);
					
					success = sendto(sockUDP, "Invalid command", 15,  
					   MSG_CONFIRM, (const struct sockaddr *) &cli_addr, 
						  sizeof(cli_addr));
			}
		}
		else{
			success = sendto(sockUDP, "I got your message", 18,  
					   MSG_CONFIRM, (const struct sockaddr *) &cli_addr, 
						  sizeof(cli_addr));
			if (n < 0)
				error("ERROR writing to socket");
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