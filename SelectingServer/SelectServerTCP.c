/* SelectServerTCP.c - Select server.

Modified by: Nathan Dunne
Team: Robert Walsh - K00209111 | Nathan Dunne - K00211819 | Luke O Brien - P11011180
*/

#include "ServerManager.h"

// Default port to use if none is specified.
char* DEFAULT_PORT = "48040";

int main(int argc, char **argv)
{

	// Check for arguments. Only one is possible, a port number.
	if (argc > 2)
	{
		exitWithSystemMessage("1 argument possible, port number.\n\n");
	}

	// Use the arguments passed in from the command line, requires nothing.
	char *service = (argc == 2) ? (argv[1]) : DEFAULT_PORT;

	// Create the server socket/file descriptor.
	int server_file_descriptor = 0;
	server_file_descriptor = createTCPServerSocket(service);

	// If the socket creation failed.
	if (server_file_descriptor < 0)
	{
		exitWithSystemMessage("Socket creation failed. Exiting..\n\n");
	}

	// Introduce game and show application.
	printf("Welcome to Hangman at the C Arcade - Server.\n");
	printf("Waiting for players..\n");

	// Seed random with time, for use in generating words.
	srand(time(NULL));

	// Set the last, or ending, file descriptor to the descriptor of the server.
	int file_descriptor_amount = 0;
	file_descriptor_amount = server_file_descriptor;

	// Initiate a record of the ending index.
	int ending_player_index = -1;

	// Set up an array of players of FD_SETSIZE (1023)
	struct Player players[FD_SETSIZE];
	resetFileDescriptors(players); // Reset the file descriptors in the Player structure.

	// Storage for ready players and the main file descriptor set.
	fd_set ready_set, file_descriptor_set;

	// Zero out the file descriptor set.
	FD_ZERO(&file_descriptor_set);

	// Set the server file descriptor bit in the file descriptor set.
	FD_SET(server_file_descriptor, &file_descriptor_set);

	int ready_player = 0;
	int working_descriptor = 0;

	// In a psuedo infinite loop execution.
	while (true)
	{
		// Initialise the ready set to the main file descriptor set.
		ready_set = file_descriptor_set;

		// Without blocking, select a player to see if they are ready to communicate with the server.
		// Pass in NULL for the writing file descriptors, the error file descriptors and the timeout structure.
		// I could not get timeouts to work.

		ready_player = select(file_descriptor_amount + 1, &ready_set, NULL, NULL, NULL);
		//printf("Unprocessed players: %d\n", unprocessedPlayers);

		int new_file_descriptor;

		struct sockaddr_in playerSocketAddress;

		// If the bit for the server file descriptor is set in the file descriptor set pointed to by the ready set of players.
		if (FD_ISSET(server_file_descriptor, &ready_set))
		{
			socklen_t PlayerLength = sizeof(playerSocketAddress);

			// Accept the connection, and set the file descriptor.
			new_file_descriptor = accept(server_file_descriptor, (struct sockaddr*) &playerSocketAddress, &PlayerLength);

			bool is_new_fd_found = false;
			int emptyDescriptor = -1;

			int	i = 0;

			// Find a new player structure to use in the set.
			for (i = 0; i < FD_SETSIZE && !is_new_fd_found; i++)
			{
				// Only use if the file descriptor of the player is empty.
				if (players[i].file_descriptor == emptyDescriptor)
				{
					// Start their session.
					beginPlayerSession(new_file_descriptor, &players[i]);

					bool is_player_game_over = false;

					// Send the current progress to the player, at this stage the word will all be blank underscores ("_____").
					sendProgressToPlayer(&players[i], new_file_descriptor, is_player_game_over);

					// Use to break out of the loop.
					is_new_fd_found = true;
				}
			}

			// Set the bit for the new file descriptor in the main descriptor set.
			FD_SET(new_file_descriptor, &file_descriptor_set);

			// If the new file descriptor increases the overall set size.
			if (new_file_descriptor > file_descriptor_amount)
			{
				// Set the amount of file descriptors to the new file descriptor, which is the largest index in that set.
				file_descriptor_amount = new_file_descriptor;
			}

			// Set where to end the search within the player structures based on where the last new player joined.
			if (i > ending_player_index)
			{
				ending_player_index = i;
			}
		}

		int	i = 0;

		// For every player connected to the server
		for (i = 0; i < ending_player_index+1; i++)
		{
			// Set the working descriptor to the file descriptor of the player.
			working_descriptor = players[i].file_descriptor;

			// If that working descriptor is active in the set of players who are ready to input to the server.
			if (FD_ISSET(working_descriptor, &ready_set))
			{
				//printf("Active descriptor.");
				if (players[i].player_state == 'I')
				{
					// Read the input.
					ssize_t number_of_bytes_read = recv(working_descriptor, players[i].input_from_player, MAX_BUFFER, 0);

					// If no input is recieved there is a problem and the session must be terminated.
					if (number_of_bytes_read == 0)
					{
						// Terminate the player session.
						terminatePlayer(working_descriptor, &players[i]);
					}
					else // If input is present and valid.
					{
						// Terminate the string at the second index, only want to use first entry.
						// TODO: Use this as away for the player to enter an "exit" command.
						players[i].input_from_player[1] = '\0';

						// Check the input against the game logic of hangman.
						checkPlayerGuess(&players[i]);

						// Update the state of the player after their guess has been processed.
						updatePlayerState(&players[i]);

						// Check to see if the player is in the game over state.
						bool is_player_game_over = isGameOverForPlayer(&players[i]);

						// If the game is over.
						if (is_player_game_over == 1)
						{
							// Send back the last message to the player. As the game is over at this point, the full word is sent back.
							sendProgressToPlayer(&players[i], working_descriptor, is_player_game_over);

							// Terminate the player session.
							terminatePlayer(working_descriptor, &players[i]);

							// Clear the working descriptor bit from the file descriptor set.
							FD_CLR(working_descriptor, &file_descriptor_set);
						}
						else
						{
							// Send the word in progress, as normal, to player.
							sendProgressToPlayer(&players[i], working_descriptor, is_player_game_over);
						}
					}
				}
			}
		}
	}
}