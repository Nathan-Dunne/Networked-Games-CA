// written by Luke O Brien
// clientUDP
// team members: Luke O Brien, Nathan Dunne and Robert Walsh

#include <stdio.h>						
#include <string.h>							
#include <stdlib.h>							
#include <unistd.h>							
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include "HangmanUDP.h"		

// the default port and server ip (local)
#define PORT "4200"								
#define SERVER_IP "127.0.0.1"							

// function definitions
int UDP_SocketCreation();
struct sockaddr_in AddressStructureSetup(char* server_name, char* port);

int main(int argc, char * argv[])
{
	struct sockaddr_in  serverAddress;
	int socket;
	int incomingBytes;
	int lives;					
	int slen = sizeof(serverAddress);					
	char guess[LENGTH];
	char partword[LENGTH];


	// creating a socket and setting up the address structure for sending data to the server
	socket = UDP_SocketCreation();
	serverAddress = AddressStructureSetup((argc == 2) ? argv[1] : SERVER_IP, (argc == 3) ? argv[2] : PORT);


	printf("Welcome to Hangman!\n");
	printf("Please Enter Your First Guess To Start The Game\n");	

	//getting the players guess and sending to the server
	fgets(guess, LENGTH, stdin);				
	sendto(socket, guess, strlen(guess), 0, (struct sockaddr *) &serverAddress, slen);

	while (1)
	{
		//reset the partword
		partword[0] = '\0';

		//read in the partword from the server
		incomingBytes = recvfrom(socket, &partword, LENGTH, 0, (struct sockaddr *) &serverAddress, &slen);

		//exit the game when the server sends @
		if (partword[0] == '@') break;
		//char *pExitCheck = strchr(partword, '-');
		//if (pExitCheck == NULL) break;			


		// manage the word and lives	
		char word[LENGTH];
		//get the partword and lives from the server
		sscanf(partword, "%s %d", &(*word), &lives);		

		printf("Remaining Guesses: %d\n", lives);	

		if (lives > 0)
		{
			printf("Word To Guess: %s\n", word);
		}
		else
		{
			printf("The word was: %s\n", word);
		}

		//getting the players guess and sending to the server
		fgets(guess, LENGTH, stdin);			
		sendto(socket, guess, strlen(guess), 0, (struct sockaddr *) &serverAddress, slen);
	}

	// end game message
	if (lives > 0)
	{
		printf("You Won!\n");
	}
	else
	{
		printf("You Lose! Better Luck Next Time\n");
	}

	//close the connection and exit the program
	close(socket);							
	exit(0);							

	return 0;
}

//creating a socket
int UDP_SocketCreation()
{
					
	int sock;

	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) 
	{
		printf("Socket Creation Failed\n");			
	}

	return sock;							


// setting up the address structure (IPv4)
struct sockaddr_in AddressStructureSetup(char* server_name, char* port)
{
	struct sockaddr_in serverAddress;					

	memset((char *)&serverAddress, 0, sizeof(serverAddress));			

	serverAddress.sin_family = AF_INET;						
	serverAddress.sin_port = htons(atoi(port));				

	inet_aton(server_name, &serverAddress.sin_addr) 
			
	return serverAddress;							
}

