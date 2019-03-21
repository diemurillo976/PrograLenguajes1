#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>

//for generating random usernames
extern char* words[100];
extern char digits[10];

//Username data
struct infoCard {
	char userId[40];        //username
	unsigned short colorId; //color
};

struct user{
	struct infoCard info;
	short online;
	struct sockaddr_in addr;
};

//global user list
extern struct user* users;

// UTILS
void error(const char *); // Prints an error msg and exits the program
int getRandom(int,int); // Creates a random integer value between the provided arguments
char* generateId(char*); // Picks a random username and a random 2-digit number
unsigned short generateColor(); // Picks a random color id

// SOCKET CONFIG
int getPortNumber(); // Gets the port number from config file
void getIPAddress(char*); // Gets the IP from config file

// USER FUNCTIONS
short verifyUserAssignation(char *user); // Verifies if user enters a username on client startup
int findExistingClient(char*); // Verifies if a user already exists in server

char *getUser(char *, char *); // Gets user on client startup message
void getUserIdFromMessage(char *, char *); // Gets the recipient user ID from a message
short isCommand(char*); // Checks if a message is a command. Format: "comm _command symbol_"
void getMessage(char *, char *); // Gets the message body

void addOnlineUser(struct infoCard*, struct sockaddr_in*); // Adds a new user to the user list
struct user* findOnlineUser(char*); // Gets if a specified userID is currently online
struct user* findUserByAddress(struct sockaddr_in*); // Gets a user info corresponding to a specified address
void removeOnlineUser(char*); // Sets a specified userID as offline

void printOnlineUsers(); // Prints a list of online users on console
void addOnlineUsersToMessage(char*); // Prints a list of online users onto a message

// FORK FUNCTION
void standByMe(int); // Client listening function

#endif //SERVER_H
