Game Server and Client

Overview

This project includes a game server and client. The server manages game logic and client connections, while the client interacts with the server.

Requirements

- C Compiler (e.g., gcc)
- Socket Programming (Unix-like systems or Windows with appropriate libraries)

Running the Server

1. Compile the Server:

   gcc -o game_server game_server.c

2. Run the Server:

   ./game_server <Port Number> <Game Type> <Game Arguments>

   Example:

   ./game_server 5555 numbers 3

   This starts the server on port 5555 with the game type numbers and an argument of 3.

Running the Client

1. Compile the Client:

   gcc -o game_client game_client.c

2. Run the Client:

   ./game_client <Server IP> <Port Number> <Game Type>

   Example:

   ./game_client 127.0.0.1 5555 numbers

   This connects the client to the server at 127.0.0.1 on port 5555 with the game type numbers.

Instructions for Playing

1. Start the Server before starting the client.
2. Run the Client and follow the prompts to play the game.
3. Use the quit command to exit the game.

Troubleshooting

- Server Issues: Ensure no other application is using the port and check server logs.
- Client Issues: Verify server IP and port, and ensure the server is running.

