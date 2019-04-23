// written by Luke O Brien
// HangmanUDP
// team members: Luke O Brien, Nathan Dunne and Robert Walsh

#ifndef	__HANGMAN
#define	__HANGMAN

#include <errno.h>
#include <syslog.h>


#define NUM_OF_WORDS (sizeof (word) / sizeof (word [0]))			
#define LIVES 10								
#define LENGTH 128

// function definitions
void PlayHangman(int in, int out);
int IsGuessRight(char* wholeWord, char* partWord, char* guess);
int examineGuess(char* outputBuffer, char* wholeWord, char* partWord, char* guess, int* lives);
char IsGameOver(char* wholeWord, char* partWord, int lives);


//read in the words text file as an array
char* word[] = {	
#include "words"	
};

// setting the address structure and its size 
struct sockaddr_in clientAddress;	
int slen = sizeof(clientAddress);

// the main component of the hangman game (edited the original)
void PlayHangman(int in, int out) 
{
	//variable declaration
	char* whole_word;
	char* part_word[LENGTH];
	char* guess[LENGTH];
	char* outputBuffer[LENGTH];

	int lives = LIVES;
	int game_state = 'I';
	int i;
	int word_length;

	//get the hostname and handle it
	char hostname[LENGTH];
	gethostname(hostname, LENGTH);									
	sprintf(outputBuffer, "Playing hangman on host %s: \n\n", hostname);
	write(out, outputBuffer, strlen(outputBuffer));

	
	whole_word = word[rand() % NUM_OF_WORDS];
 	word_length = strlen(whole_word);
	syslog(LOG_USER | LOG_INFO, "server chose hangman word %s", whole_word);
	printf("The chosen word is %s\n", whole_word);

	//setting up the unsolved word that the client will see
	for (i = 0; i < word_length; i++)
	{
		part_word[i] = '-';
	}
	//terminating the string
	part_word[i] = '\0';					

	while (1) 
	{		
		//getting the partword and lives
		sprintf (outputBuffer, "%s\n %d \n\n", part_word, lives);					
		sendto(out, outputBuffer, LENGTH, 0, (struct sockaddr *) &clientAddress, slen);		
		
		//is the game over?
		if ((game_state != 'I'))
		{
			break;							
		}

		//get the guess from the client
		if (recvfrom(in,guess,LENGTH,0, (struct sockaddr *) &clientAddress, &slen) == -1) 
		{	
			printf("Recieve Failed\n");	
			break;
		}

		//check if the guess is correct
		examineGuess(outputBuffer, whole_word, part_word, guess, &lives);

		//if all of the lives are gone
		if ((game_state = IsGameOver(whole_word, part_word, lives)) == 'L')	
		{
			//show the client the word
			strcpy(part_word, whole_word); 
		}
								
		// print the remaining lives
		printf("Lives left: %d\n", lives);	
		
	}	

	// using an @ symbol to signal the end of the game
	sprintf (outputBuffer, "%s\n", "@");							
	sendto(out, outputBuffer, LENGTH, 0, (struct sockaddr *) &clientAddress, slen);			

	close(in);										
	exit(0);
}



int IsGuessRight(char* wholeWord, char* partWord, char* guess) 
{

	int i;
	int good_guess = 0;
	//checks to see if the clients guess matches any of the characters in the chosen word
	for (i = 0; i < strlen(wholeWord); i++) 
	{
		if (guess[0] == wholeWord[i])
		{						
			good_guess = 1;			
			//overwrite the guessed character
			partWord[i] = wholeWord[i];							
		}
	}

	return good_guess;								
}		



int examineGuess(char* outputBuffer, char* wholeWord, char* partWord, char* guess, int* lives) 
{
	// if the guess was wrong
	if (!IsGuessRight(wholeWord, partWord, guess)) 
	{			
		// decrement the clients lives
		(*lives)--;										 
		sprintf(outputBuffer, "Bad Guess Received From Client  %s\n", guess);
	}
	else
	{
		sprintf(outputBuffer, "Good Guess Received From Client  %s\n", guess);
	}
	// write the result to the servers output
	write(0, outputBuffer, strlen(outputBuffer));								

	//return the number of remaining lives
	return (*lives);							
}


char IsGameOver(char* wholeWord, char* partWord, int lives) 
{
	// if all the letters were guessed then the client won, if the client has no lives then they lost
	if (strcmp(wholeWord, partWord) == 0)
	{
		return 'W'; 			
	}
	else if (lives <= 0)
	{
		return 'L';					
	}

	//otherwise the game is still in progress
	return 'I';
}

#endif	// __HANGMAN
