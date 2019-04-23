// written by Luke O Brien
// serverUDP
// team members: Luke O Brien, Nathan Dunne and Robert Walsh

#include <stdio.h>
#include <string.h>			
#include <stdlib.h>			
#include <time.h>			
#include <unistd.h>			
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include "HangmanUDP.h"	


#define PORT "4200"								

// function definition
int UDP_SocketCreation(char* port);


int main (int argc, char * argv []) 
{									
	int socket;								

	// create a socket with the passed in details or use the default
	socket = UDP_SocketCreation((argc == 2) ? argv[1] : PORT);					

	printf("Welcome to Hangman!\n");

	while(1) 
	{
		// flush the output buffer
		fflush(stdout);	

		// seeding random based on time
		srand(time(NULL));	

		//start the game
		PlayHangman(socket,socket);
		printf("Finished Playing Hangman\n");			

		//close the connection
		close(socket);							
	}	

	return 0;	
}


int UDP_SocketCreation(char* port) 
{
	struct sockaddr_in serverAddress;
	int serverSocket;

	//create a socket,
	if ((serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		printf("The Socket Creation Has Failed\n");				
	}

	//set the address to 0
	memset((char *)&serverAddress, 0, sizeof(serverAddress));				

	//set up the socket as IPv4 and the correct port
	serverAddress.sin_family = AF_INET;						
	serverAddress.sin_port = htons(atoi(port));						
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);				

	// bind the socket to the server address
	if (bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1)
	{
		printf("The Bind Has Failed\n");
	}
						
	return serverSocket;								
}


