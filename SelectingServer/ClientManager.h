/* ClientManager.h - Utility functions for the client.

Modified by: Nathan Dunne
Team: Robert Walsh - K00209111 | Nathan Dunne - K00211819 | Luke O Brien - P11011180
*/

// Include guards.
#ifndef	__CLIENT_MANAGER
#define	__CLIENT_MANAGER

// Standard libraries.
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

// BSD libraries.
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

// The max allowed size of sends and recieves.
#define MAX_BUFFER 127

// Input and output buffers.
char output_to_server[MAX_BUFFER];
char input_to_player[MAX_BUFFER];

// Handle unintentional behaviour.
void exitWithSystemMessage(const char* message)
{
	perror(message);
	exit(1);
}


// Setup a socket for the client.
int setupTCPClientSocket(const char *host, const char *service)
{
	struct addrinfo address_criteria;
	// Zero out the structure.
	memset(&address_criteria, 0, sizeof(address_criteria));

	// Family is unspecified, for dual stack IPv4 or IPv6.
	address_criteria.ai_family = AF_UNSPEC;

	// Using streaming with TCP.
	address_criteria.ai_socktype = SOCK_STREAM;
	address_criteria.ai_protocol = IPPROTO_TCP;

	// Get the address info.
	struct addrinfo *server_address; // List of server addresses.
	int address_error = getaddrinfo(host, service, &address_criteria, &server_address);

	// If we can't find any address to use.
	if (address_error != 0)
	{
		exitWithSystemMessage("getaddrinfo() failed");
	}

	int client_socket = -1;

	// For any applicable address found from getaddrinfo.
	for (struct addrinfo *address = server_address; address != NULL; address = address->ai_next)
	{
		// Create the socket to communicate on.
		client_socket = socket(address->ai_family, address->ai_socktype, address->ai_protocol);

		// Socked creation failed, try next address.
		if (client_socket < 0)
		{
			continue;
		}

		// Socket creation success. Attempt to connect and break from the loop if successful.
		if (connect(client_socket, address->ai_addr, address->ai_addrlen) == 0)
		{
			break;
		}

		// If connect fails, close the socket and try next address.
		close(client_socket);

		// Reset the socket.
		client_socket = -1;
	}

	// Free the memory allocated to the list of server addresses.
	freeaddrinfo(server_address);

	// Return the socket for use.
	return client_socket;
}

// Update the player on the progress of the game.
int getProgress(char* server_input)
{
	char word_in_progress[MAX_BUFFER];
	int remaining_lives;

	// Format scan for a string (full word) and the integer (remaining lives) from the server input.
	sscanf(server_input, "%s %d", &(*word_in_progress), &remaining_lives);

	// Inform the player if their lives are low or not.
	if (remaining_lives < 3 && remaining_lives > 0)
	{
		printf("Guess the word! \nLow lives remaining: %d\n", remaining_lives);
	}
	else
	{
		printf("Guess the word! \nLives remaining: %d\n", remaining_lives);
	}

	// Player is continuing.
	if (remaining_lives > 0)
	{
		printf("%s\n", word_in_progress);
	}
	// If the player has no more lives the server will send the full word instead of the word in progress, so give a different print out here.
	else
	{
		printf("Correct word was: %s\n", word_in_progress);
		printf("Press enter to get results and exit. \n"); // Prompt exit.
	}

	return remaining_lives;
}

// Inform the player at the end of the game.
void giveResult(int lives)
{
	if (lives > 0)
	{
		printf("You win. :) Now exiting...\n");
	}
	else
	{
		printf("You lose. :( Now exiting...\n");
	}
}

// Play the hangman game, alternating between server input and player output.
void playGame(int socket)
{
	int lives = 0;
	int bytes_read = 0;

	// Introduce game and show application.
	printf("Welcome to Hangman at the C Arcade - Player.\n");

	// In a psuedo infinite loop execution.
	while (true)
	{
		// Read the input from the server, with no flags.
		bytes_read = recv(socket, input_to_player, MAX_BUFFER, 0);

		// If there is nothing from the server, end the loop execution.
		if (bytes_read == 0)
		{
			//printf("Breaking.\n");
			break;
		}
		else
		{
			// Determine the game state (lives of player) and update them on their progress,
			lives = getProgress(input_to_player);

			// Read in from the keyboard.
			fgets(output_to_server, MAX_BUFFER, stdin);

			// Send output to the server, with no flags.
			send(socket, output_to_server, strlen(output_to_server), 0);
		}
	}

	// Show the player their ending state (win or lose).
	giveResult(lives);
}

#endif // End include guards.