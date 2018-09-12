#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h> 
#define SIZE 150000

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int socketFD;
	int portNumber;
	int charsWritten;
	int charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[SIZE];
	char messageKeyChecker[] = "&"; // Separates the message and key
	char endOfFile[] = "e"; // Checker for the server that the message is completely sent
    
	if (argc < 4) 
	{
		fprintf(stderr,"USAGE: %s hostname port\n", argv[0]);
		 exit(0);
	}

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) 
	{
		fprintf(stderr, "CLIENT: ERROR, no such host\n");
		exit(0);
	}
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) 
	{
		error("CLIENT: ERROR opening socket");
	}	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
	{
		error("CLIENT: ERROR connecting");
	}

	// Create FILE variables for message and key
	FILE *messageFile;
	FILE *keyFile;
	char messageArray[SIZE];
	// Memset the message array
	memset(messageArray, '\0', sizeof(messageArray));
	char keyArray[SIZE];
	// Memset the key array
	memset(keyArray, '\0', sizeof(keyArray));
	// Open message file
	messageFile = fopen(argv[1], "r");
	// Open key file
	keyFile = fopen(argv[2], "r");
	// Place message file into single string
	fgets(messageArray, SIZE, messageFile);	
	// Get size of message array
	int messageSize = strlen(messageArray);
	// Place key file into a single string 
	fgets(keyArray, SIZE, keyFile);
	// Get size of message array
	int keySize = strlen(keyArray);
	// Prints error if it is too short
	// Testing length
	if(messageSize > keySize)
	{
		perror("ERROR, key is too short");
		exit(1);
	}
	
	//Check if message file has all properly letters - TO DO
	//
	// Reset the buffer
	memset(buffer, '\0', sizeof(buffer));
	// Place message into the buffer - strip off newline
	strncpy(buffer, messageArray, strlen(messageArray) - 1);
	// Add null terminator in the message
	//buffer[messageSize] = '\0';	
	//Send message to server
	charsWritten = send(socketFD, buffer, strlen(buffer), 0);

	if (charsWritten < 0)
	{
		error("CLIENT: ERROR writing to socket");
	}
	
	if(charsWritten != strlen(buffer))
	{
		error("CLIENT: ERROR not all chars written");
	}

	// Place separator in between message and key - &
	charsWritten = send(socketFD, messageKeyChecker, 1, 0);
	// Error Checking
	if (charsWritten < 0)
	{
		error("CLIENT: ERROR writing to socket");
	}
	// Reset the buffer
	memset(buffer, '\0', sizeof(buffer));
	// Place key into the buffer - Strip off newline
	strncpy(buffer, keyArray, strlen(messageArray) - 1);
	// Add null terminator in the key
	//buffer[messageSize] = '\0';
	// Send key to server
	charsWritten = send(socketFD, buffer, strlen(buffer), 0);

	if (charsWritten < 0)
	{
		error("CLIENT: ERROR writing to socket");
	}
	if(charsWritten != strlen(buffer))
	{
		error("CLIENT: ERROR not all chars written");
	}
	// Send and 'e' for the end of file flag for the server
	charsWritten = send(socketFD, endOfFile, 1, 0);
	// Error Checking
	if (charsWritten < 0)
	{
		error("CLIENT: ERROR writing to socket");
	}
	// Get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	// Get message from server
	int k;
	while(1)
	{
		charsRead = recv(socketFD, buffer, strlen(messageArray) + 1, MSG_WAITALL); // Read data from the socket, leaving \0 at end
		usleep(50);
		//printf("CLIENT: Inside loop charsRead: %d\n", charsRead);
		if (charsRead < 0) 
		{
			error("CLIENT: ERROR reading from socket");
		}
		usleep(50);
		for(k = 0; k < charsRead; k++)
		{
			if(buffer[k] == 'z')
			{
				break;
			}
		}
		if(k != charsRead)
		{
			break;
		}			
		usleep(50);	
	}
	//printf("CLIENT: charsRead: %d\n", charsRead);
	//int i;
	//for(i = 0; i < charsRead; i++)
	//{
		//printf("%c", buffer[i]);
	//}	
	//printf("CLIENT: printing buffer: %s\n", buffer);	
	//printf("CLIENT: printing buffer-1: %s\n", buffer-1);
		
	// Print to stdout - could go to file - placed in newline again
	char* message;
	message = strtok(buffer, "z");	
	
	fprintf(stdout, "%s\n", message);	
	
	close(socketFD); // Close the socket
	return 0;
}


/*References
 * http://man7.org/linux/man-pages/man2/lseek.2.html
 *
 */
