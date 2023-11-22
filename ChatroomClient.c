/*
Chat Client Project

-Implemented socket for client to connect to server
-Added error catching if a user does not input correct command parameters (server host & port)
-Added a command parameter for "chat room name", so that a client can specify the chat room
 it wants to join. ex: "first", "Lamar student chat", "COSC 4333 chat"
-Implemented a way to leave a chat room (using the word "Bye" ~case sensitive)

Things to do:
-Clean up code
    review and update variable names to be more descriptive
    review and update function names to be more descriptive
    review error catching
    add block documentation for functions and variables

*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// Function prototype
int call_socket(char*, int);

int main(int argc, char* argv[])
{
    int socket, port;
    char buf[256]; /* buffer for data from the server */

    /* Check for correct number of command-line arguments */
    if (argc == 4) {
        port = atoi(argv[2]);
    }
    else {
        fprintf(stderr, "Usage: %s server-hostname port chat-room\n", argv[0]);
        exit(1);
    }

    socket = call_socket(argv[1], port);

    while (1)
    {
        int length; /* number of characters that have been read */

        bzero(buf, 256);
        fgets(buf, 256, stdin);
        if (length = write(socket, buf, strlen(buf) + 1) < 0) {
            fprintf(stderr, "Error on writing");
        }
        bzero(buf, 256);
        if (length = read(socket, buf, 256) < 0) {
            fprintf(stderr, "Error on reading");
        }

        printf("Server: %s", buf);

        int i = strncmp("Bye", buf, 3);
        if (i == 0)
            break;
    }

    /* Repeatedly read data from socket and write to user.s screen.
    charactersRead = read(socket, buf, sizeof(buf));
    while (charactersRead > 0) {
        write(1, buf, charactersRead);
        printf("n: %d\n", charactersRead);
        charactersRead = recv(socket, buf, sizeof(buf), 0);
    }
    printf("n: %d\n", charactersRead);

    /* Close the socket. */
    close(socket);
    return 0;
}

int call_socket(char* hostname, int portnum)
{
    struct sockaddr_in serverAddress;
    struct hostent* server;
    int s;

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) { /* get socket */
        fprintf(stderr, "socket creation failed\n");
        exit(1);
    }

    if ((server = gethostbyname(hostname)) == NULL) {    /* do we know the host's */
        fprintf(stderr, "invalid host: %s\n", hostname);       /* no */
        exit(1);
    }

    serverAddress.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&serverAddress.sin_addr.s_addr, server->h_length);
    serverAddress.sin_port = htons(portnum);
    //memset(&serverAddress, 0, sizeof(serverAddress));
    //memcpy((char*)&serverAddress.sin_addr, server->h_addr, server->h_length); /* set address */

    if (connect(s, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) { /* connect */
        close(s);
        fprintf(stderr, "connect failed\n");
        exit(1);
    }

    return(s);
}