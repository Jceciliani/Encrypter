#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

// Get size to fit lengths
#define SIZE 150000

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

// Get int from specific upper case char or space, 1 - 27
int getInt(char letter)
{
	const char *charList = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

	int i;
	for(i = 0; i < 27; i++)
	{
		if(charList[i] == letter)
		{
			return i;
		}		
	}
}

char getChar(int letter)
{
	const char *charList = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
	
	return charList[letter];
}

// Encrypt the message sent in with the key given
void encrypt(char msg[], char key[], int len)
{
	int messageNumber; // Number from the message
	int keyNumber; // Number from the key
	int encNumber; // Var used to combine both numbers, and mod 27 to get new number	
	//len = (strlen(msg)); // Length of the message - 1 to account for newline

	int i;
	for(i = 0; i < len; i++)
	{
		// Make ints from the chars in both message and key
		messageNumber = getInt(msg[i]);
		keyNumber = getInt(key[i]);
		// Combine the numbers and mod 27(number of elements)
		encNumber = (messageNumber + keyNumber) % 27;
		// Place new char in message
		msg[i] = getChar(encNumber); 
	}
	return;
}

int main(int argc, char *argv[])
{
	int listenSocketFD;
	int establishedConnectionFD;
	int portNumber;
	int charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[SIZE];
	char endOfFile[] = "z";
	struct sockaddr_in serverAddress, clientAddress;

	// Check usage of args
	if (argc < 2) 
	{ 
		fprintf(stderr,"USAGE: %s port\n", argv[0]);
		exit(1); 
	} 

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) 
	{
		error("ERROR opening socket");
	}
	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
	{	
		error("ERROR on binding");
	}
	
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections
	while(1)
	{
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) 
		{
			error("ERROR on accept");
		}
		// Fork off child once connection is accepted
		pid_t childId = fork();
		// If fork errors - display
		if(childId <  0)
		{
			error("ERROR on fork");
		}	

		// If child is successful
		if(childId == 0)
		{
			// Clear out buffer again for next round of inputs and usage	
			memset(buffer, '\0', SIZE);
			// Variables to help keep track of reading the whole message
			int totalBytes = 0;
			int readBytes = 0;
			int j;			
			// Get message and place in buffer
			//printf("SERVER: going into read all of the bytes\n"); fflush(stdout);
			while(1)
			{	
				// Find the number of bytes read this iteration
				readBytes = recv(establishedConnectionFD, buffer, 1000, 0);
				if(readBytes < 0)
				{
					error("ERROR reading from socket");
				}
			        // Get updated total of bytes
				totalBytes = totalBytes + readBytes;

				for(j = 0; j < readBytes; j++)
				{
					// Break out if at the end of the file
					if(buffer[j] == 'e')
					{
						break;
					}

				}
				if(j != readBytes)
				{
					break;
				}
			}
			//printf("SERVER: Readbytes is: %d\n", readBytes); fflush(stdout);
			//printf("SERVER: Finished reading all of the bytes\n"); fflush(stdout);
			//printf("SERVER: Print buffer: %s\n", buffer); fflush(stdout);
			//printf("SERVER: String length of buffer: %d\n", strlen(buffer)); fflush(stdout);
			//int i;
			//for(i = 0; i < 76; i++)
			//{
			//	printf("%c", buffer[i]); fflush(stdout);
			//}
			//printf("\n"); fflush(stdout);
			// Create message variable
			char* message;
			// Create key variable
			char* key;
			// Clear out key variable
			//printf("SERVER: Tokenizing the string\n"); fflush(stdout);
			// Get message
			message = strtok(buffer, "&");
			//printf("SERVER: Message is: %s\n", message); fflush(stdout);
			// Get key
			key = strtok(NULL, "e");
			//printf("SERVER: Key is: %s\n", key); fflush(stdout);
			//printf("SERVER: Encrypting message\n"); fflush(stdout);
			//encrypt the message with the key
			encrypt(message, key, strlen(message));
			// Get length of message to key track of sending	
			int messageLength = strlen(message);
			//printf("SERVER: Encrypted message: %s\n", message); fflush(stdout);	
			//printf("SERVER: Encrypted message length: %d\n", strlen(message)); fflush(stdout);
			// Send encrypted message back to client
			//printf("SERVER: Sending message to client\n"); fflush(stdout);			
			charsRead = send(establishedConnectionFD, message, strlen(message), 0);			
			charsRead = send(establishedConnectionFD, endOfFile, 1, 0);
			//printf("SERVER: Sent message\n"); fflush(stdout);
			// Error Check
			if(charsRead < 0)
			{		
				error("ERROR writing to socket");
			}
		}

		close(establishedConnectionFD); // Close the existing socket which is connected to the client
	}	
	close(listenSocketFD); // Close the listening socket
	return 0; 
}

