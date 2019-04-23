/* ServerManager.h - Utility functions for the server.

Modified by: Nathan Dunne
Team: Robert Walsh - K00209111 | Nathan Dunne - K00211819 | Luke O Brien - P11011180
*/

// Include guards.
#ifndef	__SERVER_MANAGER
#define	__SERVER_MANAGER

// Standard libraries.
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// BSD libraries.
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>


// Neat #include trick to quickly add a list of words in a text file to a string array.
char* list_of_words[] =
{
  #include "WordList"
};

// The max allowed size of sends and recieves.
#define MAX_BUFFER 127

// Structure to hold player data and progress/state.
struct Player
{
	int player_state; // Can be I, W or L.
	int remaining_lives;
	int file_descriptor; // The file descriptor to read on.

	char* full_word;
	char word_in_progress[MAX_BUFFER];

	char input_from_player[MAX_BUFFER];
	char output_to_player[MAX_BUFFER];
};


// Find if an character entry is in a word. Runs O(n), can be improved with a binary search.
bool isEntryInWord(char* entry, char* word_in_progress, char* full_word)
{
	bool is_entry_in_word = false;
	int word_length = strlen(full_word);
	int i = 0;

	// For each letter in the word.
	for (i = 0; i < word_length; i++)
	{
		// If the entry is somewhere in the word array.
		if (full_word[i] == entry[0])
		{
			is_entry_in_word = true;

			// Unveil more of the word in progress if the entry was found in the full word.
			word_in_progress[i] = full_word[i];
		}
	}

	// Return success or failure for player guess.
	return is_entry_in_word;
}

// Check the player guess input to see if its correct, if not deduct a life from them.
void checkPlayerGuess(struct Player *player)
{
	printf("\nPlayer %d input letter: %s\n", (*player).file_descriptor, (*player).input_from_player);

	// If the entry is found in the full word.
	if (isEntryInWord((*player).input_from_player, (*player).word_in_progress, (*player).full_word))
	{
		printf("Player %d input is correct.\n", (*player).file_descriptor);
	}
	else // If not, deduct lives.
	{
		(*player).remaining_lives += -1;
		printf("Player %d input is incorrect, taking away lives..\n", (*player).file_descriptor);
	}

	// Show the remaining player lives.
	printf("Player %d remaining lives: %d \n", (*player).file_descriptor, (*player).remaining_lives);
}

// Check to see if the player is in progress or won or lost and update their state.
void updatePlayerState(struct Player* player)
{
	// If the word they have constructed through guesses is the same as the word stored at the start.
	if (strcmp((*player).full_word, (*player).word_in_progress) == 0)
	{
		// Player wins.
		(*player).player_state = 'W';

		printf("Player %d state is now W.\n", (*player).file_descriptor);
	}
	else if ((*player).remaining_lives < 1)
	{
		// Player loses.
		(*player).player_state = 'L';
		printf("Player %d state is now L.\n", (*player).file_descriptor);
	}
	else
	{
		// Player is still in progress.
		(*player).player_state = 'I';
		printf("Player %d state is I.\n", (*player).file_descriptor);
	}
}

// Check to see if the player is in the game over state.
bool isGameOverForPlayer(struct Player* player)
{

	// Assumption that the game is not over.
	bool is_game_over = false;

	// If they have won, the game is over.
	if ((*player).player_state == 'W')
	{
		printf("Player %d has guessed the full word.\n", (*player).file_descriptor);
		is_game_over = true;
	}
	else if ((*player).player_state == 'L') // If they have lost, the game is over.
	{
		printf("Player %d is out of remaining lives.\n", (*player).file_descriptor);
		is_game_over = true;
	}

	// If they haven't won or lost, the game is still not over.
	return is_game_over;
}

// Helper function to clear a player structure of all their data.
void clearPlayer(struct Player* player)
{
	// Reset the file descriptor and the lives.
	(*player).file_descriptor = -1;
	(*player).remaining_lives = 10;

	// Reset the full word to empty and state to inital.
	(*player).full_word = "";
	(*player).player_state = 'I';

	// Terminate all the dynamic strings for safety.
	(*player).word_in_progress[0] = '\0';
	(*player).input_from_player[0] = '\0';
	(*player).output_to_player[0] = '\0';

	//printf("Clearing player data.\n");
}

// Pseduo random word generation from the list of pre-compiled words found in WordList.txt
char* giveNewRandomWord()
{
	// Find the amount of words in the list.
	int word_amount = (sizeof(list_of_words) / sizeof(list_of_words[0]));

	// Mod rand() by the amount of words.
	char* new_word = list_of_words[rand() % word_amount];

	return new_word;
}

// Inform the player of their current game progress.
void sendProgressToPlayer(struct Player* player, int working_descriptor, bool is_game_over)
{
	int number_of_bytes;

	// If the game is over, set their word in progress to the full word for them to see at the end.
	if (is_game_over)
	{
		strcpy((*player).word_in_progress, (*player).full_word);
	}

	// Explicitly format a string that contains a string(player progress) and an integer(player lives).
	sprintf((*player).output_to_player, "%s %d \n", (*player).word_in_progress, (*player).remaining_lives);

	// Send the progress output to the player through the file descriptor given.
	number_of_bytes = send(working_descriptor, (*player).output_to_player, strlen((*player).output_to_player), 0);

	// Print player progress to the server.
	if (is_game_over)
	{
		printf("Player %d was shown full word:  \"%s\"\n", (*player).file_descriptor, (*player).word_in_progress);
	}
	else
	{
		printf("Player %d word in progress:  %s\n", (*player).file_descriptor, (*player).word_in_progress);
	}

	// Print out the bytes sent to the player.
	//printf("Bytes sent to player %d: %d\n\n",(*player).file_descriptor, number_of_bytes);
}

// Initiate a player session/connection.
void beginPlayerSession(int file_descriptor, struct Player* player)
{
	// Set the player file descriptor, print out results.
	(*player).file_descriptor = file_descriptor;
	printf("\nNew player connected, file descriptor: %d\n", (*player).file_descriptor);

	// Set initial state.
	(*player).player_state = 'I';

	// Set lives for  the player.
	(*player).remaining_lives = 10;

	// Generate a random word for use in the game.
	(*player).full_word = giveNewRandomWord();
	printf("Player %d was given the word: \"%s\" to guess!\n", (*player).file_descriptor, (*player).full_word);

	// Loop over the word in progress and fill it with underscores.
	int char_index;
	for (int char_index = 0; char_index < strlen((*player).full_word); char_index++)
	{
		if ((char_index) != strlen((*player).full_word))
		{
			(*player).word_in_progress[char_index] = '_';
		}
		else // Make sure to terminate the string at the end of the word in progress.
		{
			(*player).word_in_progress[char_index] = '\0';
		}
	}
}

// Handle unintentional behaviour.
void exitWithSystemMessage(const char* message)
{
	perror(message);
	exit(1);
}

// Setup a socket for the server.
int createTCPServerSocket(char* service)
{
	struct addrinfo address_criteria;
	// Zero out the structure.
	memset(&address_criteria, 0, sizeof(address_criteria));

	// Family is unspecified, for dual stack IPv4 or IPv6.
	address_criteria.ai_family = AF_UNSPEC;

	// Accept on any address/port
	address_criteria.ai_flags = AI_PASSIVE;

	// Using streaming with TCP.
	address_criteria.ai_socktype = SOCK_STREAM;
	address_criteria.ai_protocol = IPPROTO_TCP;

	// Get the address info.
	struct addrinfo *server_address; // List of server addresses.
	int address_error = getaddrinfo(NULL, service, &address_criteria, &server_address);

	// If we can't find any address to use.
	if (address_error != 0)
	{
		exitWithSystemMessage("getaddrinfo() failed");
	}

	int server_socket = -1;

	// For any applicable address found from getaddrinfo.
	for (struct addrinfo *address = server_address; address != NULL; address = address->ai_next)
	{
		// Create the socket to communicate on.
		server_socket = socket(address->ai_family, address->ai_socktype, address->ai_protocol);

		// Socked creation failed, try next address.
		if (server_socket < 0)
		{
			continue;
		}

		// Max pending connections.
		const int MAXPENDING = 5;

		// Socket creation success. Attempt to bind, listen and break from the loop if successful.
		if ((bind(server_socket, address->ai_addr, address->ai_addrlen) == 0) &&
			(listen(server_socket, MAXPENDING) == 0))
		{
			printf("Setup socket on port: %d\n", atoi(service));

			break;
		}

		// If bind and or listen fails, close the socket and try next address.
		close(server_socket);

		// Reset the socket.
		server_socket = -1;
	}

	freeaddrinfo(server_address);

	return server_socket;
}

// Make sure all file descriptors are empty before starting.
void resetFileDescriptors(struct Player players[])
{
	int i = 0;
	int emptyDescriptor = -1;

	// FD_SETSIZE is a constant 1023, should not attempt to be bypassed.
	for (i = 0; i < FD_SETSIZE; i++)
	{
		// Make sure all potential players start with an empty descriptor for allocation, showing them as available for use.
		players[i].file_descriptor = emptyDescriptor;
	}
}

// Terminate the player session and connection.
void terminatePlayer(int working_descriptor, struct Player* player)
{
	printf("Terminated connection to Player %d. \n", (*player).file_descriptor);

	// Close the socket/descriptor.
	close(working_descriptor);

	// Clear the player structure for later re-use.
	clearPlayer(player);
}

#endif // End include guards.