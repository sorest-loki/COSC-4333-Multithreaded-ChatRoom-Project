/*
Chatroom Client Project

Shahanze Sabri
Roberto Rivera
Javian Zeno

2023 Fall COSC 4333 - Distributed Systems
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>

// Function prototypes
int isCommandLineCorrect(int);
int isPortValid(char*);
int createAndConnectSocket(char*, int);
char* getUsername();
void* readAndPrint(void* socketFd);

int main(int argc, char* argv[])
{
    int socketFd, port;
    char buf[1000]; // buffer for storing the string sent between clients and server
    char* username; // Used for displaying the name of a user sending a message
    char message[1000]; // Used for the concatenation of username + buffer

    if (isCommandLineCorrect(argc) && isPortValid(argv[2]))
        port = atoi(argv[2]);

        socketFd = createAndConnectSocket(argv[1], port);

        printf("Connection to Server was successful\n");

        // Prompt and Save user's name
        username = getUsername();

        printf("Type the name of a Chatroom you would like to join and press enter: ");
        bzero(buf, 1000);
        fgets(buf, 1000, stdin);
        buf[strlen(buf) - 1] = '\0'; // Remove newline char from end of string

        // write the name of chatroom to server main
        if (write(socketFd, buf, strlen(buf) + 1) < 0) {
            fprintf(stderr, "Error on writing chatroom name to server\n");
            exit(1);
        }

        pthread_t id;
        pthread_create(&id, NULL, &readAndPrint, &socketFd);

    while (1)
    {
        // Clear buffers and perform next write operation
        bzero(buf, 1000);
        bzero(message, 1000);
        strncpy(message, username, 50);
        fgets(buf, 1000, stdin);
        strcat(message, buf);
        if (write(socketFd, message, strlen(message) + 1) < 0) {
            fprintf(stderr, "Error with writing to the client socket\n");
        }

        int i = strncmp("Bye", buf, 3);
        if (i == 0)
            break;
    }

    // Close the socket
    free(username);
    close(socketFd);

    return 0;
}

// Check whether the number of command-line arguments is correct
int isCommandLineCorrect(int argc)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: ./Client (Server Hostname) (Port)\n");
        exit(1);
    }
    return 1;
}

// The port must be above 1024. 1 - 1024 are reserved by the system. There are no ports above 65535.
int isPortValid(char* arg)
{    
    int port = atoi(arg);

    if (port <= 1024 || port > 65535) {
        fprintf(stderr, "The port number entered is invalid\n", arg);
        exit(1);
    }
    return 1;
}

// Function builds a socket, gets the IP of the server, and attempts a connection to the server.
int createAndConnectSocket(char* hostname, int port)
{
    struct sockaddr_in address; // Structure to hold the server address
    // The hostent structure gives access to (int h_addrtype) Address type,
    // and (int h_length) the length, in bytes, of the address.
    struct hostent* server; 
    int fd; // Client file descriptor

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { // Attempt socket creation
        fprintf(stderr, "socket creation failed\n");
        exit(1);
    }

    // The next several lines of code construct the socket address
    address.sin_family = AF_INET;
    address.sin_port = htons(port); // host to network short function - used to account for little/big Endian

    if ((server = gethostbyname(hostname)) == NULL) { // Check that the host is known
        fprintf(stderr, "invalid host: %s\n", hostname);
        exit(1);
    }

    // Copies the byte sequence from server into the address
    bcopy((char*)server->h_addr, (char*)&address.sin_addr.s_addr, server->h_length);

    if (connect(fd, (struct sockaddr*)&address, sizeof(address)) < 0) { // Attempt socket connect
        close(fd);
        fprintf(stderr, "connect failed\n");
        exit(1);
    }

    return(fd);
}

// Function to prompt and store the username that will display to other clients in a chat group.
char* getUsername()
{
    char* username = (char*)malloc(50);

    printf("Type your user name: ");

    fgets(username, 50, stdin);
    username[strlen(username) - 1] = '\0'; // Remove newline char from end of string
    strcat(username, "> ");

    return username;
}

// Function that continuously performs a read operation on a separate thread to print messages to client.
void* readAndPrint(void* socketFd)
{
    int threadSocketFd;
    threadSocketFd = *(int*)socketFd;
    char buff[1000];

    // Clear any data in the buffer
    bzero(buff, 1000);

    while (read(threadSocketFd, buff, 1000) > 0) {

        printf("%s", buff);
    }

    close(threadSocketFd);
}
