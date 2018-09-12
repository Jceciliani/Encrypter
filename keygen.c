/* Author: Josh Ceciliani
 * Title: Keygen
 * Date: 8/13/2018
 * Description: This program randomly generates a key to use for encrytion and decrytion.
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char* argv[])
{
	// Randomize the numbers
	srand(time(NULL));

	char randchar; // int to get the random char into
	int lengthOfKey; // get the length of the key
	
	// Check the number of arguments, if it is under 2, send error to stderr and return 1
	if(argc < 2)
	{
		fprintf(stderr, "Error: to few arguments\n");
		exit(1);		
	}	

	// Must have an atoi cast. Conversion from string to int
	lengthOfKey = atoi(argv[1]);

	int i;
	for(i = 0; i < lengthOfKey; i++)
	{
		// Randomize chars based on upper case letters - 27 adds the inclusion of a space - idea taken from Stack Overflow
		randchar = "ABCDEFGHIJKLMNOPQRSTUVWXYZ "[rand() % 27];
		// print to stdout
		fprintf(stdout, "%c", randchar);
	}

	// Add a newline per requirement specifications
	fprintf(stdout, "%c", '\n');

	return 0;
}

//References
//https://stackoverflow.com/questions/19724346/generate-random-characters-in-c
//

