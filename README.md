# COSC-4333-Multithreaded-ChatRoom-Project

This project contains a server program and a client program.

Both programs are based on Unix/Linux operating systems.

Before the programs can run, first save both files to an accessible directory.
Open a new terminal, and go to the directory which the files are located.
The files must be compiled first.
To compile, use the following commands on the terminal screen:
    gcc ChatroomServer.c -pthread -o Server
    gcc ChatroomClient.c -o Client

Two executables will now be present in the directory- named Server and Client.
Because this is a Chatroom application, multiple terminals will need to be opened
to run the client program repeatedly.

To run the server program, use the command:
    ./Server (Port)
Provide any valid port number between 1025-65535.

To run the client program, use the command:
    ./Client (IP Address) (Port)
The client program will abruptly exit if the server program is not already running.
