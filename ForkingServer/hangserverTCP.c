/*  Network server for hangman game 
    Concurrent server that utilises the fork function to handle multiple clients

    File: hangserverTCP.c 
    Made by: Robert Walsh
    Team: Robert Walsh - K00209111 | Nathan Dunne - K00211819 | Luke O Brien - P11011180 
*/

 #include <sys/types.h>
 #include <sys/socket.h>
 #include <sys/wait.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <string.h>
 #include <syslog.h>
 #include <signal.h>
 #include <errno.h>

 extern time_t time ();
 int play_hangman (int, int);
 char clientIP[INET_ADDRSTRLEN];
 int maxlives = 12;

 // Create array of words from words text file
 char *word [] = {
 # include "words"
 };

 # define NUM_OF_WORDS (sizeof (word) / sizeof (word [0]))
 # define MAXLEN 80 
 # define HANGMAN_TCP_PORT 1066
 # define CLI_PORT ntohs(client.sin_port)

 int main ()
 {
 	int sock, fd, client_len, process_id;
	void catch_sigchld(int);
 	struct sockaddr_in server, client;

	// Create IPv4 TCP stream socket. 0 uses the default protocol
 	sock = socket (AF_INET, SOCK_STREAM, 0);

	// The socket function returns a value that can be used for error checking
 	if (sock < 0)
	{ 
 		perror ("Error creating stream socket\n");
 		exit (1);
 	}
	else { printf("Created stream socket\n"); }

	// Set the protocol, IP address and port number for the server structure
 	server.sin_family = AF_INET;
 	server.sin_addr.s_addr = htonl(INADDR_ANY);
 	server.sin_port = htons(HANGMAN_TCP_PORT);

	/* Bind the socket to an address and check for errors
	   Arg 1 is the file descriptor, the address to which it is bound is arg 2
	   Arg 3 is the size of the address */
 	if (bind(sock, (struct sockaddr *) & server, sizeof(server)) < 0) 
	{
 		perror ("Error binding socket\n");
	 	exit (2);
 	}
	else { printf("Binded socket"); }

	// Listen for incoming connections. 5 is the size of the backlog queue
 	listen (sock, 5);

	// Passing 1 results in an infinite loop
 	while (1) 
	{
		// Get the size of the client socket
 		client_len = sizeof (client);

		/* Block until a client connects to the server
		   Arg 1 is the file descriptor. Arg 2 is a reference to client address
		   Arg 3 is the size of the structure.*/
 		if ((fd = accept (sock, (struct sockaddr *) &client, &client_len)) < 0) 
		{
 			perror("\n\nError accepting connection\n");
 			exit (3);
 		}
		else { printf("\n\nConnection accepted\n"); }

		// Randomize the seed
		srand ((int) time ((long *) 0));
		
		// Create a child process and save its process id
		process_id = fork();
		
		// Convert IP address to string
		inet_ntop(AF_INET, &client.sin_addr.s_addr, clientIP, sizeof(clientIP));

		// Display IP address, Port and Process ID. Adding pid check prevents line from printing twice
		if(clientIP != NULL && process_id != 0)
		{
			printf("Handling client\nIP address: %s Port number: %d Process ID: %d\n", clientIP, CLI_PORT, process_id);
		}

		// On successful fork, close listening socket, play hangman and exit on finish
		if (process_id == 0)
		{
			printf("Created child\n");
 			close (sock);
			play_hangman (fd, fd);
			printf("\n\nChild process terminated\nIP address: %s Port number: %d Process ID: ", clientIP, CLI_PORT);
			exit(0);
		}
		else if(process_id < 0) { perror("Error creating child\n"); }	

		//Close the client connection
		close (fd);	

		// Sets catch_sigchld function as handler for signal
		signal(SIGCHLD, catch_sigchld);		
 	}
 }

// Catch SIGCHLD signal from terminating child to prevent zombie processes
void catch_sigchld(int signum)
{
	int process_id, status;
	
	/* Waitpid suspends execution of calling process until the child has terminated
	   Passing -1 waits for any child process. WNOHANG returns immediately if no child has 		   
	   exited. Waitpid info is stored in status */

	while((process_id = waitpid(-1, &status, WNOHANG)) > 0)
	{
		printf("%d \n", process_id);
	}

	return;
}


 /* ---------------- Play_hangman () ---------------------*/

 int play_hangman (int in, int out)
 {
 	char * whole_word, part_word [MAXLEN],
 	guess[MAXLEN], outbuf [MAXLEN];

 	int lives = maxlives;
 	int game_state = 'I';//I = Incomplete
 	int i, good_guess, word_length;
 	char hostname[MAXLEN];

 	gethostname (hostname, MAXLEN);
 	sprintf(outbuf, "Playing hangman on host %s: \n \n", hostname);
 	write(out, outbuf, strlen (outbuf));

 	/* Pick a word at random from the list */
 	whole_word = word[rand() % NUM_OF_WORDS];
 	word_length = strlen(whole_word);
 	syslog (LOG_USER | LOG_INFO, "server chose hangman word %s", whole_word);

 	/* No letters are guessed Initially */
 	for (i = 0; i <word_length; i++)
 		part_word[i]='-';
 	
	part_word[i] = '\0';

 	sprintf (outbuf, "%s %d \n", part_word, lives);
 	write (out, outbuf, strlen(outbuf));

 	while (game_state == 'I')
 	/* Get a letter from player guess */
 	{
		while (read (in, guess, MAXLEN) <0) {
 			if (errno != EINTR)
 				exit (4);
 			printf ("re-read the startin \n");
 			} /* Re-start read () if interrupted by signal */
 	good_guess = 0;
 	for (i = 0; i <word_length; i++) {
 		if (guess [0] == whole_word [i]) {
 		good_guess = 1;
 		part_word [i] = whole_word [i];
 		}
 	}
 	if (! good_guess) lives--;
 	if (strcmp (whole_word, part_word) == 0)
 		game_state = 'W'; /* W ==> User Won */
 	else if (lives == 0) {
 		game_state = 'L'; /* L ==> User Lost */
 		strcpy (part_word, whole_word); /* User Show the word */
 	}
 	sprintf (outbuf, "%s %d \n", part_word, lives);
 	write (out, outbuf, strlen (outbuf));
 	}
 }
