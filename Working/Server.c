#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <pthread.h> 
#include <sys/mman.h>
#include <arpa/inet.h>

char* words[100] ={"abecedarian","abracadabra","accoutrements","adagio","aficionado","agita","agog","akimbo","alfresco","aloof","ambrosial","amok","ampersand","anemone","anthropomorphic","antimacassar","aplomb","apogee","apoplectic","appaloosa","apparatus","archipelago","atingle","avuncular","azure","babushka","bailiwick","bafflegab","balderdash","ballistic","bamboozle","bandwagon","barnstorming","beanpole","bedlam","befuddled","bellwether","berserk","bibliopole","bigmouth","bippy","blabbermouth","blatherskite","blindside","blob","blockhead","blowback","blowhard","blubbering","bluestockings","boing","boffo (boffola)","bombastic","bonanza","bonkers","boondocks","boondoggle","borborygmus","bozo","braggadocio","brainstorm","brannigan","breakneck","brouhaha","buckaroo","bucolic","buffoon","bugaboo","bugbear","bulbous","bumbledom","bumfuzzle","bumpkin","bungalow","bunkum","bupkis","burnsides","busybody","cacophony","cahoots","calamity","calliope","candelabra","canoodle","cantankerous","catamaran","catastrophe","catawampus","caterwaul","chatterbox","chichi","chimerical","chimichanga","chitchat","claptrap","clishmaclaver","clodhopper","cockamamie","cockatoo","codswallop"};
char digits[10] = {'0','1','2','3','4','5','6','7','8','9'};

int getRandom(int,int);
char* generateId(char*);

struct infoCard{
	char userId[40];
	int   socket;
	
};

struct user{
	struct infoCard info;
	short online;
	struct sockaddr_in addr;
};




int *expe = NULL;
struct user* users = NULL;

void addOnlineUser(struct infoCard* info, struct sockaddr_in* address){
	int i;
	for (i = 0; i < 100; i++){
		if((*(users+i)).online == 0){
			strcpy((*(users+i)).info.userId, (*info).userId);
			(*(users+i)).info.socket = (*info).socket;
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
			printf("user %s %d \n",(*(users+i)).info.userId,(*(users+i)).info.socket);
		}
	}
}

void standByMe(int, char*, int);
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
    
    users = mmap(NULL, sizeof(struct user)*100, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	int i;
	for (i = 0; i < 100; i++){
		(*(users+i)).online = 0;	
		bzero((char *) &((*(users+i)).addr), sizeof((*(users+i)).addr));
	}
    
	expe = malloc(sizeof(int));
    int socketFileDescriptor;
	int socketUDP;
    int newSocketFileDescriptor;
    int portNumber; //Port number on which the server accepts connections
    int pid;
    socklen_t clientAddressLength; //size of the address of the client
	struct sockaddr_in servUDP_addr;
    struct sockaddr_in serv_addr, cli_addr; //Direccion del servidor y del cliente
    int n;	//return value for the read() and write() calls
	
    if (argc < 2){
    	fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

	socketUDP = socket(AF_INET, SOCK_DGRAM, 0); 
    if (socketUDP < 0) 
       error("ERROR opening UDP server socket");

    socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0); //creates a new socket
    if (socketFileDescriptor < 0) 
       error("ERROR opening socket");
	
	bzero((char *) &servUDP_addr, sizeof(servUDP_addr));
    bzero((char *) &serv_addr, sizeof(serv_addr)); //sets all values in buffer to 0. Initializes serv_addr to zeros
    portNumber = atoi(argv[1]); //converts the port passed as argument to an integer
	
	
	//Setting parametes of the struct sockaddr_in
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portNumber); //IP address of the host

	servUDP_addr.sin_family = AF_INET;
    servUDP_addr.sin_addr.s_addr = INADDR_ANY;
    servUDP_addr.sin_port = htons(portNumber + 1); 

	
	//bind() = binds a socket to an address, assings a name to an unnamed socket. Return 1 on success
	//The address of the current host and port number on which the server will run
	//3 arguments: socket file descriptor, address to which is bound, size of the address to which is bound
    if (bind(socketFileDescriptor, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
             error("ERROR on binding");

	if (bind(socketUDP, (struct sockaddr *) &servUDP_addr, sizeof(servUDP_addr)) < 0) 
             error("ERROR on binding UDP server");

	 

	
	//listen() = allows the process to listen on the socket for connections
	//2 arguments: socket file descriptor, number of connections that can be waiting while the process is handling a particular connection
	//set to 5 -> maximum size permitted by most systems
    listen(socketFileDescriptor,5);
	
    clientAddressLength = sizeof(cli_addr);
	
	//To get the host name
	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);
	printf("Hostname: %s\n", hostname);

	while(1)
	{
		
		newSocketFileDescriptor = accept(socketFileDescriptor, (struct sockaddr *) &cli_addr, &clientAddressLength);
		if (newSocketFileDescriptor < 0) 
			error("ERROR on accept");
		

		char* myId = malloc( (size_t)35 );
		generateId(myId);

		struct infoCard myInfo;
		strcpy(myInfo.userId,myId);
		myInfo.socket = newSocketFileDescriptor;

		char tempBuf[5]; 
     
		int lenUDP; 
		recvfrom(socketUDP, (char *)tempBuf, 256,  
		            MSG_WAITALL, ( struct sockaddr *) &cli_addr, 
		            &lenUDP); 
		
		printf("UDP client socket connected\n"); 
		int lene=20;
		char buffere[lene];

		inet_ntop(AF_INET, &(cli_addr.sin_addr), buffere, lene);
		printf("address:%s\n",buffere);

		addOnlineUser(&myInfo, &cli_addr);

		*expe = newSocketFileDescriptor;
		pid = fork();
		if(pid < 0)
			error("ERROR on fork");
		if(pid == 0){
			//close(socketFileDescriptor);
			standByMe(newSocketFileDescriptor, myId, socketUDP);
			exit(0);
		}
		//else
			//close(newSocketFileDescriptor);
	}
	close(socketFileDescriptor);
	return 0;
}

void standByMe(int mySock, char* myId, int sockUDP){

	//srand(getpid());//reseeds the randomgenerator

	

	int n;
	char buffer[256];

	


	printf("Welcome: %s %d \n", myId, *expe);
	//myId = write(mySock, "Welcome", 7);
	
	short breakUpFlag = 0;
	
	while (!breakUpFlag){
		bzero(buffer,256);
		n = read(mySock,buffer,255);
		if (n < 0)
			error("ERROR reading from socket");
		printf("Here is the message: %s\n",buffer);
		
		if (isCommand(buffer)){
			char command = buffer[5];
			
			switch (command){
				case 'e':
					breakUpFlag = 1;
					removeOnlineUser(myId);
					n = write(mySock, "$", 1);
					
					break;
				case 'g':
					printf("printing new id %d \n", *expe);
					char ss[35] = "";
					generateId(ss);
					n = write(mySock, ss, strlen(ss));
					
					break;

				case 'p':
					printf("printing online users \n");
					printOnlineUsers();
					n = write(mySock, "printing list on server", 23);
					
					break;

				case 's':
					
					printf("writing to socket %d :%d \n", buffer[7]-'0', (*(users+(buffer[7]-'0'))).addr.sin_family);
					char* hello = "que perro";
					int lene=20;
					char buffere[20];
					
					inet_ntop(AF_INET, &((*(users+(buffer[7]-'0'))).addr.sin_addr), buffere, lene);
					printf("address:%s %d\n",buffere,(int) strlen(hello));
					

					int success = sendto(sockUDP, (const char *)hello, 9,  
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
					n = write(mySock,"Invalid command",15);
			}
		}
		else{
			n = write(mySock,"I got your message",18);
			if (n < 0)
				error("ERROR writing to socket");
		}
		
	}
	close(mySock);
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














