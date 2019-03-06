#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

struct infoCard{
	char* userId;
	int   socket;
	
};


struct UserNode
{ 
    // Any data type can be stored in this node 
    void  *data; 
  
    struct UserNode *next; 
}; 

/* A linked list node */
/* Function to add a node at the beginning of Linked List. 
   This function expects a pointer to the data to be added 
   and size of the data type */
void push(struct Node** head_ref, void *new_data, size_t data_size) 
{ 
    // Allocate memory for node 
    struct Node* new_node = (struct Node*)malloc(sizeof(struct Node)); 
  
    new_node->data  = malloc(data_size); 
    new_node->next = (*head_ref); 
  
    // Copy contents of new_data to newly allocated memory. 
    // Assumption: char takes 1 byte. 
    int i; 
    for (i=0; i<data_size; i++) 
        *(char *)(new_node->data + i) = *(char *)(new_data + i); 
  
    // Change head pointer as new node is added at the beginning 
    (*head_ref)    = new_node; 
} 
  
/* Function to print nodes in a given linked list. fpitr is used 
   to access the function to be used for printing current node data. 
   Note that different data types need different specifier in printf() */
void printList(struct Node *node, void (*fptr)(void *)) 
{ 
    while (node != NULL) 
    { 
        (*fptr)(node->data); 
        node = node->next; 
    } 
} 

void remove(struct Node** head_ref, void  *data) 
{ 
   struct Node* actual_temp = *head_ref;
   struct Node* previous_temp;
   
   while(actual_temp != NULL);
    { 
        if (actual_temp->data == data){
        	if (actual_temp == *head_ref){
        		*head_ref = temp->next;   // Changed head 
		        free(temp);               // free old head 
		        
			}
			else{
				previous_temp->next = actual_temp->next; 
  
    			free(temp);  // Free memory 
			}
			return; 
		}
        
        previous_temp = actual_temp;
        actual_temp = actual_temp->next;
    }
} 


void standByMe(int);
short isCommand(int, char*);
void error(const char *msg)
{
	perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    signal(SIGCHLD,SIG_IGN);//prevents zombie processes
	
    int socketFileDescriptor;
    int newSocketFileDescriptor;
    int portNumber; //Port number on which the server accepts connections
    int pid;
    socklen_t clientAddressLength; //size of the address of the client

    struct sockaddr_in serv_addr, cli_addr; //Direccion del servidor y del cliente
    int n;	//return value for the read() and write() calls
	
    if (argc < 2){
    	fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0); //creates a new socket
    if (socketFileDescriptor < 0) 
       error("ERROR opening socket");
	
    bzero((char *) &serv_addr, sizeof(serv_addr)); //sets all values in buffer to 0. Initializes serv_addr to zeros
    portNumber = atoi(argv[1]); //converts the port passed as argument to an integer
	
	//Setting parametes of the struct sockaddr_in
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portNumber); //IP address of the host
	
	//bind() = binds a socket to an address, assings a name to an unnamed socket. Return 1 on success
	//The address of the current host and port number on which the server will run
	//3 arguments: socket file descriptor, address to which is bound, size of the address to which is bound
    if (bind(socketFileDescriptor, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
             error("ERROR on binding");
	
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
		pid = fork();
		if(pid < 0)
			error("ERROR on fork");
		if(pid == 0){
			close(socketFileDescriptor);
			standByMe(newSocketFileDescriptor);
			exit(0);
		}
		else
			close(newSocketFileDescriptor);
	}
	close(socketFileDescriptor);
	return 0;
}

void standByMe(int mySock){
	int n;
	char buffer[256];
	bzero(buffer,256);
	short breakUpFlag = 0;
	
	while (!breakUpFlag){
		
		n = read(mySock,buffer,255);
		if (n < 0)
			error("ERROR reading from socket");
		printf("Here is the message: %s\n",buffer);
		
		if (isCommand(n, buffer)){
			char command = buffer + 5;
			
			switch (command){
				case 'e':
					breakUpFlag = 1;
					break;
				default:
					printf("Invalid command!: %d", command);
			}
		}
		else{
			n = write(mySock,"I got your message",18);
			if (n < 0)
				error("ERROR writing to socket");
		}
		
	}
	
}

//checks for command input. Format: "comm _command symbol_"
short isCommand(int n, char* input){
	if (n >= 6 &&*(input) == 'c' && *(++input) == 'o' && *(++input) == 'm' && *(++input) == 'm' && *(++input) == ' '){
		return 1;
	}
	return 0;
}
