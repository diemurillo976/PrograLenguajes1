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
unsigned short generateColor();
int getPortNumber();
short verifyUserAssignation(char *user);
char *getUser(char *, char *);
void getUserIdFromMessage(char *, char *);
unsigned short getColorId(char *);
void getMessage(char *, char *);
void getIPAddress(char*);
int findExistingClient(char*);
void addOnlineUsersToMessage(char*);

struct infoCard {
	char userId[40];
	unsigned short colorId;
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
			(*(users+i)).info.colorId = (*info).colorId;
			(*(users+i)).addr = *address;
			(*(users+i)).online = 1;
			return;
		}
	}
}

struct user* findOnlineUser(char* userId){
	int i;
	for (i = 0; i < 100; i++) {
		if((*(users+i)).online == 1 && strcmp((*(users+i)).info.userId, userId) == 0)
			return users+i;
	}
	return NULL;
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

			if(strcmp(bufferUser, bufferTemp) == 0 && ((*(users+i)).addr.sin_port == (*address).sin_port)) {
				return (*(users+i)).info.userId;
			}
		}
	}
}

void removeOnlineUser(char* userId) {
	struct user* downUser = findOnlineUser(userId);

	(*(downUser)).online = 0;
}

void printOnlineUsers() {
	int i;
	int len = 20;
	char buffer[len];


	for (i = 0; i < 100; i++) {
		if((*(users+i)).online == 1) {
			printf("Username:\033[%dm %s \033[0m \n", ((*(users+i)).info.colorId), ((*(users+i)).info.userId));
			inet_ntop(AF_INET, &((*(users+i)).addr.sin_addr), buffer, len);
			printf("Address: %s\n", buffer);
			int puerto = ntohs((*(users+i)).addr.sin_port);
			printf("Port: %d\n", puerto);
			printf("\033[97m-------------------------------------\033[0m \n");
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

	int socketUDP,
	 	socketGeneral,
     	portNumber, //Port number on which the server accepts connections
     	pid,
	 	random;
	char ipAddress[15];
	bzero(ipAddress, 15);
    socklen_t clientAddressLength; //size of the address of the client
	struct sockaddr_in servUDP_addr, servUDPGeneral_addr, cli_addr; //Direccion del servidor y del cliente

    int n;	//return value for the read() and write() calls

	if (argc > 1){
    	fprintf(stderr,"\n\033[91m!!> ERROR, port provided <!!\033[0m \n");
        exit(1);
    }

	socketUDP = socket(AF_INET, SOCK_DGRAM, 0); //Peticiones de conectarse
    if (socketUDP < 0)
       error("\n\033[91m!!> ERROR opening UDP server socket <!! \033[0m \n");

	socketGeneral = socket(AF_INET, SOCK_DGRAM, 0); //Escuchar cada cliente
    if (socketGeneral < 0)
       error("\n\033[91m!!> ERROR opening general UDP server socket <!! \033[0m \n");

	bzero((char *) &servUDP_addr, sizeof(servUDP_addr));
	bzero((char *) &servUDPGeneral_addr, sizeof(servUDPGeneral_addr));
    portNumber = getPortNumber(); //gets the port number from the config file

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
    	error("\n\033[91m!!> ERROR on binding UDP server <!! \033[0m \n");

	if (bind(socketGeneral, (struct sockaddr *) &servUDPGeneral_addr, sizeof(servUDPGeneral_addr)) < 0)
        error("\n\033[91m!!> ERROR on binding general UDP server <!! \033[0m \n");

    clientAddressLength = sizeof(cli_addr);

	//To get the host name
	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);
	printf("\033[1;95m#== Hostname: %s \n", hostname);

	int lenLocalAddr=20;
	char bufferLocalAddr[lenLocalAddr];
	inet_ntop(AF_INET, &(servUDP_addr.sin_addr), bufferLocalAddr, lenLocalAddr);
	printf("#== Address: %s \n", bufferLocalAddr);
	printf("#== Port: %d \n#=-=-=-=-=-=-=-=-=-=-=# \033[0m \n", portNumber);

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
		char *myId = malloc( (size_t) 50 );
		if (assignUser) {
			generateId(myId);
			strcpy(myInfo.userId,myId);
		}else{
			char myUser[50];
			bzero((char *) myUser,sizeof(myUser));
			myId = getUser(tempBuf, myUser);
			strcpy(myInfo.userId,myId);
			existingClient = findExistingClient(myId);//Verifica si ya existe el usuario
		}
		strcpy(myInfo.userId, myId);
		if (findExistingClient(myId)) { //verifica si ya existe un usuario
			sendto(socketUDP, "1", 2, MSG_CONFIRM, (const struct sockaddr *) &cli_addr,
				   sizeof(cli_addr));
			continue;
		}
		else
			sendto(socketUDP, "0", 2, MSG_CONFIRM, (const struct sockaddr *) &cli_addr,
				   sizeof(cli_addr));

		myInfo.colorId = generateColor();

		printf("\n\033[97m---------=[ New client connected ]=---------\n");
		int lene=20;
		int portNewClient = 0;
		char buffere[lene];
		printf("User: %s\n",myId);
		inet_ntop(AF_INET, &(cli_addr.sin_addr), buffere, lene);
		printf("Address: %s\n",buffere);
		portNewClient = ntohs(cli_addr.sin_port);
		printf("port: %d\n--------------------------------------------\n", portNewClient);

		addOnlineUser(&myInfo, &cli_addr);
		printf("Username:\033[%dm %s \033[97m \n-------------------------------------------- \033[0m\n", myInfo.colorId, myInfo.userId);

		pid = fork();
		if(pid < 0)
			error("ERROR on fork");
		if(pid == 0){
			standByMe(socketGeneral);
			exit(0);
		}
	}
	return 0;
}

void standByMe(int sockUDP){
	struct sockaddr_in cli_addr;
	unsigned int n;
	n = sizeof(cli_addr);
	char buffer[256];
	int success;

	char arrayUsers[256];
	char finalMessageUsers[256];

	short breakUpFlag = 0;
	char *userMessage;

	while (!breakUpFlag){
		bzero(buffer,256);
		recvfrom(sockUDP, (char *)buffer, 256,
		         MSG_WAITALL, ( struct sockaddr *) &cli_addr,
		         &n);

		userMessage = findUserByAddress(&cli_addr); //user that sends message
		if (n < 0)
			error("\n\033[91m!!> ERROR reading from socket <!! \n\033[0m");

		if (isCommand(buffer)) {
			char command = buffer[5];

			switch (command) {
				case 'e':
					breakUpFlag = 1;
					printf("\033[%dm:->> %s has disconnected \033[0m\n", myInfo->colorId, myInfo->userId);
					removeOnlineUser(userMessage);
					success = sendto(sockUDP, "$", 1,
					   				 MSG_CONFIRM, (const struct sockaddr *) &cli_addr,
						  	 		 sizeof(cli_addr));
					break;

				case 'p':
					bzero(finalMessageUsers, 256);
					printf("\n\033[97m------=[ Printing online users.. ]=------ \033[0m \n");
					printOnlineUsers();
					strcat(finalMessageUsers, "Printing user list...");
					addOnlineUsersToMessage((char*) arrayUsers);
					strcat(finalMessageUsers, arrayUsers);

					success = sendto(sockUDP, finalMessageUsers, 256,
					   				 MSG_CONFIRM, (const struct sockaddr *) &cli_addr,
						  	 		 sizeof(cli_addr));

					break;

				default:
					printf("\n\033[93m> Invalid command!: %c \033[0m\n", command);

					success = sendto(sockUDP, "\033[93m> Invalid command!: %c \033[0m\n", 30,
					   				 MSG_CONFIRM, (const struct sockaddr *) &cli_addr,
						  	 		 sizeof(cli_addr));
			}
		}
		else { //Receive message from other users
			char userId[20];
			char message[256];
			char finalMessage[256];
			unsigned short colorMessage = getColorId(userMessage);
			bzero(userId, 20);
			bzero(message, 256);
			bzero(finalMessage, 256);
			getUserIdFromMessage(buffer, userId);
			getMessage(buffer, message);
			sprintf(finalMessage, "\033[4;%dm> %s:\033[0m ", colorMessage, userMessage);
			//strcpy(finalMessage, userMessage);
			//strcat(finalMessage, ": \033[0m");
			strcat(finalMessage, message);
			struct user *usuario = findOnlineUser(userId);
			success = sendto(sockUDP, (const char *) finalMessage,256,
				  MSG_CONFIRM, (const struct sockaddr *) &((*usuario).addr),
				  sizeof((*usuario).addr));
			if (success == -1)
				printf("Error sending message\n");//printf("0> \033[%dm%s \033[97m-> \033[%dm\"%s\" \033[97m}>\033[91m Error sending message \033[0m\n", myInfo->colorId, myInfo->userId, colorMessage, userMessage);
			else {
				printf("Message sent!\n");
				/*printf("test > \033[%dm %s < \033[0m \n", colorMessage, userMessage);
				printf("0> \033[%dm%s \033[97m-> \033[%dm%s \033[97m}> Message sent! \033[0m\n", myInfo->colorId, myInfo->userId, colorMessage, userMessage);*/
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

unsigned short generateColor() {
	int flag = getRandom(0, 1),
		colorId = getRandom(31, 36);
	unsigned short dest = colorId + 60*flag;
	return dest;
}

void getUserIdFromMessage(char *input, char *userId){
	int i = 0;
	char *c = input;
	char *format = strchr(input,':');

	if(format == NULL){
		printf("\n\033[91m!!> ERROR, wrong message format!\033[0m\n");
		error("\033[93m:: Usage => username: message \033[0m\n");
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

unsigned short getColorId(char* userId) {
	int i;
	unsigned short colorId = 39;
	for (i = 0; i < 100; i++) {
		if((*(users+i)).online == 1 && strcmp((*(users+i)).info.userId, userId) == 0) {
			colorId = (*(users+i)).info.colorId;
		}
	}
	return colorId;
}

void getMessage(char *input, char *message){
	int i = 0;
	char *format = strchr(input,':');
	if(format == NULL){
		printf("\n\033[91m!!> ERROR, wrong message format!\033[0m\n");
		error("\033[93m:: Usage => username: message \033[0m\n");
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
		printf("\n\033[91m!!> ERROR opening file <!!\033[0m\n");
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

int findExistingClient(char *userId) {
	if(findOnlineUser(userId) == NULL)
		return 0;
	return 1;
}

void addOnlineUsersToMessage(char *array) {
	int i;
	bzero(array, 256);

	for (i = 0; i < 100; i++){
		if((*(users+i)).online == 1){
			strcat(array,"User: ");
			strcat(array,(*(users+i)).info.userId);
			strcat(array,"\n");
		}
	}
}
