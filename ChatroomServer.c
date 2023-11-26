/*
Multithreaded Server Project

-Socket has been implemented using functions: establish and getConnection.
-main function has 5 threads allocated for client connection requests
-main function successfully creates a thread and passes socket to a thread routine
-Port number is now a command line argument/parameter
-Multiple clients can simultaneously connect to server, but clients can only echo messages to themselves


Things to do:
-Need main function (or a separate function) to identify join requests from clients by "chat room name"
	ex: "first", "gaming chat", "Lamar students chat"
	possible solution:
	Make an array of chars initialized to NULL values.
	Every request that comes in, take the passed name parameter and compare it with each array value.
	If no match, then a new thread is created and chat room	built.
	If there is a match, then server places client in appropriate chat room.
-Need main function (or a separate function) to start a new thread for a chat room if client wants to join a room
 that does not exist yet
-Need main function (or a separate function) to group incoming clients together when the clients want to join the
 same chat room
-Need every thread (chat room) to terminate when all clients are gone
-Thread routine needs implementation for receiving one message and broadcasting the message to all other clients

client sends the chat name as a string...
server main doesn't use worker thread, only separate threads
separate threads require their own socketFD and port, which server main can assign

thread names:
a Structure that holds:
chatroom name as a string
socketFD as an int
array of threads as a pthread type

clientFD -> Server main, check current thread names -> 
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
#define NUMBER_OF_CLIENTS_SUPPORTED = 5

// Function Prototypes
int isPortValid(int, char*);
int establishASocket(int);
int getConnection(int);
void* worker(void*);

// argc is the number of parameters needed to start the program 
// 2 are required for - one for hostname and one for port number
int main(int argc, char* argv[])
{
	int serverSocketFd; // Used as socket file descriptors
	int clientSocketFd;
	int port;
	int threadCounter = 0; // maybe change to 
	struct chatRooms;
	{

		pthread_t threads[NUMBER_OF_CLIENTS_SUPPORTED];
	};
	
	char buf[1000]; // buffer for storing the string sent between clients and server

	// Store the port entered from the command line
	// Function does some error checking on the user input to ensure port is acceptable
	port = isPortValid(argc, argv);

	// Make a server socket
	serverSocketFd = establishASocket(port);

	/* Server loop - start accepting incoming requests */
	while (1) {

		clientSocketFd = getConnection(serverSocketFd);
		pthread_create(&threads[threadCounter++], NULL, &worker, &clientSocketFd);

	}
	close(clientSocketFd);
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
	if (argc < 2) {
		fprintf(stderr, "Usage: %s port\n", argv[0]);
		exit(1);
	}

	// The port must be above 1024. 1 - 1024 are reserved for the system. There are no ports above 65535.
	if (argv[1] <= 1024 || argv[1] > 65535) {
		fprintf(stderr, "The port number entered is invalid\n", argv[0]);
		exit(1);
	}
	return atoi(argv[1]);
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
		fprintf(stderr, "Hostname could not be determined\n", argv[0]);
		exit(1);
	}

	// The next several lines of code construct the socket address
	&serverAddress = malloc(sizeof(struct sockaddr_in));
	serverAddress.sin_family = hostIP->h_addrtype; // this is the host address
	serverAddress.sin_port = htons(port); // host to network short function - used to account for little/big Endian

	if ((socketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) { // Attempt socket creation
		fprintf(stderr, "socket creation failed\n");
		exit(1);
	}

	if (bind(socketFD, &serverAddress, sizeof(serverAddress)) < 0) { // Attempt socket address binding
		fprintf(stderr, "bind failed\n");
		close(socketFD);
		exit(1);
	}

	if (listen(socketFD, NUMBER_OF_CLIENTS_SUPPORTED) < 0) { // Attempt socket listen operation
		fprintf(stderr, "listen failed\n");
		exit(1);
	}

	return(socketFD);
}

/* Wait for a connection to occur on a socket created with establishASocket() */
int getConnection(int socketFD)
{
	int clientSocketFD;
	struct sockaddr_in clientAddress;
	int clientAddressSize = sizeof(struct sockaddr_in);

	if ((clientSocketFD = accept(socketFD, &clientAddress, &clientAddressSize)) < 0) { // Accept connection if there is one
		fprintf(stderr, "accept failed\n");
		exit(1);
	}

	return(clientSocketFD);
}

// This is the thread routine that runs when a thread is created to execute a command //
void* worker(void* arg)
{
	int socket;
	socket = *(int*)arg;
	int	len = 0;
	char buff[256];

	// Clear any data in the buffer
	bzero(buff, 256);

	if (socket < 0) {
		fprintf(stderr, "Socket failed to pass to thread routine...\n");
		exit(1);
	}

	/* iterate, echoing all data received until end of file */
	while ((len = read(socket, buff, 256)) > 0) {
		if (len = write(socket, buff, strlen(buff) + 1) < 0) {
			fprintf(stderr, "Error in the thread. Failed to write to buffer.\n");
			exit(1);
		}

		int i = strncmp("Bye", buff, 3);
		if (i == 0)
			break;
	}

	close(socket);

	pthread_exit(0);
}