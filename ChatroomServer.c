/*
Multithreaded Server Project

-Socket has been implemented using functions: establish and get_connection.
-main function has 5 threads allocated for client connection requests
-main function successfully creates a thread and passes socket to a thread routine
-Port number is now a command line argument/parameter
-Multiple clients can simultaneously connect to server, but clients can only echo messages to themselves


Things to do:
-Implement error checking on an input port number.
	The port must be > 1024. 1-1024 are reserved by the system. There are no ports above 65535.
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

// Function Prototypes
int establish(int);
int get_connection(int);
void* worker(void*); // worker thread

/* argc is the number of parameters - 2 are required for hostname and port number */
int main(int argc, char* argv[]) // main thread, or dispatcher thread
{
	int server_s, client_s, port, i = 0;
	pthread_t threads[5]; // 5 threads to support 5 clients
	char buf[1000]; /* buffer for string the server sends */

	/* Check for proper server usage */
	if (argc < 2) {
		fprintf(stderr, "Usage: %s port\n", argv[0]);
		exit(1);
	}

	port = atoi(argv[1]);

	/* Check that the entered port number is not negative */
	if (port < 2) {
		fprintf(stderr, "Invalid port number\n");
		exit(1);
	}

	// Make a server socket
	server_s = establish(port);

	/* Main server loop - start accepting incoming requests */
	while (1) {

		client_s = get_connection(server_s);
		pthread_create(&threads[i++], NULL, &worker, &client_s);

		/* Join terminated thread //
		if (pthread_join(threads[i-1], NULL) != 0) {
			fprintf(stderr, "Error joining a thread...\n");
			exit(1);
		}

		/*visits++;
		sprintf(buf, "This server has been contacted %d time%s\n",
			visits, visits == 1 ? "." : "s.");
		send(client_s, buf, strlen(buf), 0);*/


	}
	close(client_s);
	close(server_s);

	return 0;
}

// Assist from server.c
// creates socket, binds, and listens
int establish(int portnum)
{
	char   myname[_SC_HOST_NAME_MAX + 1];
	int    s;
	struct sockaddr_in serverAddress;	/* structure to hold server address */
	struct hostent* hp;

	memset(&serverAddress, 0, sizeof(struct sockaddr_in)); /* clear our address */
	gethostname(myname, _SC_HOST_NAME_MAX);           /* who are we? */
	hp = gethostbyname(myname);                  /* get our address info */
	if (hp == NULL)                             /* we don't exist !? */
		exit(1);
	serverAddress.sin_family = hp->h_addrtype;           /* this is our host address */
	serverAddress.sin_port = htons(portnum);	/* this is our port number */

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "socket creation failed\n");
		exit(1);
	} /* create socket. No protocol such as TCP is specified */

	if (bind(s, (struct sockaddr*)&serverAddress, sizeof(struct sockaddr_in)) < 0) {
		fprintf(stderr, "bind failed\n");
		close(s);
		exit(1);                               /* bind address to socket */
	}

	if (listen(s, 5) < 0) {						/* max # of queued connects */
		fprintf(stderr, "listen failed\n");
		exit(1);
	}

	return(s);
}

/* Wait for a connection to occur on a socket created with establish() */
int get_connection(int s)
{
	int t;                  /* socket of connection */

	if ((t = accept(s, NULL, NULL)) < 0) {		/* accept connection if there is one */
		fprintf(stderr, "accept failed\n");
		exit(1);
	}

	return(t);
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