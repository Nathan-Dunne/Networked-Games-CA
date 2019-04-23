/* SelectClientTCP.c - Select client.

Modified by: Nathan Dunne
Team: Robert Walsh - K00209111 | Nathan Dunne - K00211819 | Luke O Brien - P11011180
*/

#include "ClientManager.h"

// Default port to use if none is specified.
char* DEFAULT_PORT = "48040";

int main(int argc, char **argv)
{
	int socket;

	// Test for arguments.
	if (argc > 3 || argc < 2)
	{
		exitWithSystemMessage("Requires server IP.");
	}

	// Use the arguments passed in from the command line, requires the server IP.
	char *server = argv[1];
	char *service = (argc == 3) ? (argv[2]) : DEFAULT_PORT;

	// Create the socket to communicate on.
	socket = setupTCPClientSocket(server, service);

	// If the socket isn't active or no connection could be made.
	if (socket < 0)
	{
		exitWithSystemMessage("Socket creation failed. Exiting..");
	}

	// Play the hangman game through this socket.
	playGame(socket);

	// When game is finished.
	close(socket);
	exit(0);
}