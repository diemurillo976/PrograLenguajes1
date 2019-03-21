#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>

// UTILS
void error(const char *); // Prints an error msg and exits the program
int getRandom(int,int); // Creates a random integer value between the provided arguments

// SOCKET CONFIG
int getPortNumber(); // Gets the port number from config file
int getPortNumberClient(); // Gets the Client Port number from config file
void getIPAddress(char*); // Gets the IP from config file

// MESSAGE CHECKS
short isCommand(char*); // Checks if a message is a command. Format: "comm _command symbol_"
int verifyMessageFormat(char *); // Checks if the input has a correct format (message or command)

// FORK FUNCTION
void standByYou(struct sockaddr_in*, int); //Sends and receive messages from a Server


#endif
