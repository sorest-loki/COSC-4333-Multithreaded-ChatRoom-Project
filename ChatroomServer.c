/*
Multithreaded Server Project

Roberto Rivera

2023 Fall COSC 4333 - Distributed Systems
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <limits.h>
#define NUMBER_OF_CLIENTS_SUPPORTED 5

// Function Prototypes
int isPortValid(int, char*);
int establishASocket(int);
int getConnection(int);
void handleIncomingConnection(pthread_t*, int);
void* worker(void*);
void echoToOtherClients(char*, int);

struct client
{
	char* name;
	int port;
	int socketFd;
	int ID;
};

int connectedClients[NUMBER_OF_CLIENTS_SUPPORTED];
int clientCounter = 0; // Tracks the number of concurrent clients

/*
argc is the number of parameters needed to start the program
2 are required for - one for hostname and one for port number
*/
int main(int argc, char* argv[])
{
	int serverSocketFd; // Used as socket file descriptor
	int clientSocketFd;
	int serverPort;
	pthread_t ID;
	char buf[1000]; // buffer for storing the string sent between clients and server

	// Store the port entered from the command line
	// Function does some error checking on the user input to ensure port is acceptable
	serverPort = isPortValid(argc, argv[1]);

	// Make a server socket, bind the IP address, and begin listening for connecting sockets
	serverSocketFd = establishASocket(serverPort);

	// Server loop - start accepting incoming requests
	while (1) {

		// Block incoming connections if maximum number of clients are connected
		if (clientCounter < NUMBER_OF_CLIENTS_SUPPORTED) {

			// Try connecting a new client
			clientSocketFd = getConnection(serverSocketFd);
			// Check whether getConnection() accepts client's socket or not
			if (clientSocketFd < 0) {
				fprintf(stderr, "Error on accepting client socket...\n");
				exit(1);
			}
			else {
				printf("Connection to a Client was successful\n");
			}

			// Read the name of the chatroom a client wants to join
			printf("Reading the chatroom name from client...\n");
			if (read(clientSocketFd, buf, 1000) < 0) {
				fprintf(stderr, "Failed to read chatroom name from client.\n");
				exit(1);
			}

			/*// Save the chatroom name from client and compare with existing chatroom names
			for (int i = 0; i < NUMBER_OF_CLIENTS_SUPPORTED; i++) {
				if (strncmp(connectedClients[i].name, buf, strlen(buf)) == 0) {
					// The client wants to join a room that exists
				}
			}
			*/

			// Move client to existing thread

			// Move client to a new thread that will handle read/write operations
			handleIncomingConnection(&ID, clientSocketFd);
		}
		else {
			printf("Connection limit has been reached. Please try again later.\n");
			pthread_join(ID, 0);
		}
	}

	close(serverSocketFd);

	return 0;
}

/*
This function returns an error message to the user if the command line does not include a port number.
An error is also returned to the user if the port is not acceptable.
*/
int isPortValid(int argc, char* argv)
{
	// Check for proper command line format
	if (argc != 2) {
		fprintf(stderr, "Usage: ./Server (Port)\n");
		exit(1);
	}

	int port = atoi(argv);

	// The port must be above 1024. 1 - 1024 are reserved for the system. There are no ports above 65535.
	if (port <= 1024 || port > 65535) {
		fprintf(stderr, "The port number entered is invalid\n");
		exit(1);
	}
	return port;
}

/*
Creates a socket that is returned as a file descriptor.
The socket is also binded and listens for connection requests.
Error checking is performed after each socket operation to notify the user of failure.
*/
int establishASocket(int port)
{
	char myname[_SC_HOST_NAME_MAX + 1]; // Maximum length of a hostname + a NULL character
	int socketFD; // Used to return a server file descriptor
	struct sockaddr_in serverAddress;	// Structure to hold the server address
	struct hostent* hostIP; // The hostent structure gives access to (int h_addrtype) Address type.

	gethostname(myname, _SC_HOST_NAME_MAX); // Retrieve our hostname
	hostIP = gethostbyname(myname); // Store our address info

	if (hostIP == NULL) { // Check if retrieving the hostname IP did not work
		fprintf(stderr, "Hostname could not be determined\n");
		exit(1);
	}

	// The next several lines of code construct the socket address
	serverAddress.sin_family = hostIP->h_addrtype; // this is the host address
	serverAddress.sin_port = htons(port); // host to network short function - used to account for little/big Endian

	if ((socketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) { // Attempt socket creation
		fprintf(stderr, "socket creation failed\n");
		exit(1);
	}

	if (bind(socketFD, (struct sockaddr*)&serverAddress, sizeof(struct sockaddr_in)) < 0) { // Attempt socket address binding
		fprintf(stderr, "bind failed\n");
		close(socketFD);
		exit(1);
	}

	printf("Waiting for client connection...\n");

	if (listen(socketFD, NUMBER_OF_CLIENTS_SUPPORTED) < 0) { // Attempt socket listen operation
		fprintf(stderr, "listen failed\n");
		exit(1);
	}

	return(socketFD);
}

/*
Wait for a connection to occur on a socket created with establishASocket()
Also increments a count for tracking the number of connected clients
*/
int getConnection(int socketFD)
{
	int clientSocketFD;
	struct sockaddr_in clientAddress;
	socklen_t clientAddressSize = sizeof(struct sockaddr_in);

	if ((clientSocketFD = accept(socketFD, (struct sockaddr*)&clientAddress, &clientAddressSize)) < 0) { // Accept connection if there is one
		fprintf(stderr, "accept failed\n");
		exit(1);
	}

	connectedClients[clientCounter++] = clientSocketFD;
	return(clientSocketFD);
}

/*
This function starts a thread that will handle all read/write operations between clients
*/
void handleIncomingConnection(pthread_t* ID, int clientSocketFd)
{
	pthread_create(ID, NULL, &worker, &clientSocketFd);
}

// This is the thread routine that runs when a thread is created to execute a command //
void* worker(void* clientSocketFd)
{
	int socketFd;
	socketFd = *(int*)clientSocketFd;
	char buff[1000];

	// Clear any data in the buffer
	bzero(buff, 1000);

	// iterate, echoing all data received until end of file
	while ((read(socketFd, buff, 1000)) > 0) {

		// Echo all data received to the other clients
		echoToOtherClients(buff, socketFd);

		int i = strncmp("Bye", buff, 3);
		if (i == 0)
			break;
	}

	close(socketFd);

	pthread_exit(0);
}

// Function performs a write operation to all socket file descriptors besides itself
void echoToOtherClients(char* buffer, int socketFd)
{
	for (int i = 0; i < clientCounter; i++) {

		if (socketFd != connectedClients[i]) {

			if (write(connectedClients[i], buffer, strlen(buffer) + 1) < 0) {
				fprintf(stderr, "Error when attempting to write to another client.\n");
				exit(1);
			}
		}
	}
}
