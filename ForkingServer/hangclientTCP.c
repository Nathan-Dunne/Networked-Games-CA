/*  HangclientTCP.c - Client for hangman server.

    File: hangclientTCP.c 
    Made by: Robert Walsh
    Team: Robert Walsh - K00209111 | Nathan Dunne - K00211819 | Luke O Brien - P11011180 
*/

 #include <stdio.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <netdb.h>
 #include <string.h>
 #include <stdlib.h>
 #include <unistd.h>

 # define LINESIZE 80
 # define HANGMAN_TCP_PORT 1066

 int main (int argc, char * argv [])
 {
 	struct sockaddr_in server; 
 	struct hostent * host_info;
 	int sock, count;
 	char i_line[LINESIZE];
 	char o_line[LINESIZE];
 	char *server_name;

 	// Get server name from the command line.  If none, use 'localhost'. Causes segmentation fault when passing in argument.
 	server_name = (argc == 2) ?  argv [2]: "localhost";

 	// Create the socket and check for errors
 	sock = socket (AF_INET, SOCK_STREAM, 0);

 	if (sock < 0) 
	{
 		perror ("Creating stream socket");
 		exit (1);
 	}

	// Return a pointer to a hostent containing information about the host and check for errors
 	host_info = gethostbyname(server_name);

 	if (host_info == NULL) 
	{
 		fprintf (stderr, "%s: unknown host:%s \n", argv [0], server_name);
 		exit (2);
 	}

 	// Set up the server's socket address
	// Copy host address to server.sin_addr. Arg 3 is the num of bytes to be copied
	// Convert values between host and network byte order
 	server.sin_family = host_info->h_addrtype;
 	memcpy ((char *) & server.sin_addr, host_info->h_addr, host_info->h_length);
 	server.sin_port = htons (HANGMAN_TCP_PORT);

	// Connect and check for errors
 	if (connect (sock, (struct sockaddr *) & server, sizeof server) < 0) 
	{
 		perror ("connecting to server");
 		exit (3);
 	}

	// Take a line from the server and show it, take a line and send the user input to the server. 
 	// Repeat until the server terminates the connection.

 	printf ("Connected to server %s \n", server_name);

 	while ((count = read (sock, i_line, LINESIZE)) > 0) 
	{
 		write (1, i_line, count);
 		count = read (0, o_line, LINESIZE);
 		write (sock, o_line, count);
 	}
 }
