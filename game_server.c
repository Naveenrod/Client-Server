// Naveen Rodrigo s5286336
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/select.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define MAX_ERRORS 5
#define TIMEOUT 20

int total;
int current_clients = 0;
int client_sockets[MAX_CLIENTS];

void handle_client(int client_socket, int index) {
    char buffer[BUFFER_SIZE];
    int number;
    int error_count = 0;
    int active_clients;
    struct timeval timeout;
    fd_set readfds;
    ssize_t recv_len;

    while (1) {
        // Prompt for input
        // snprintf(buffer, sizeof(buffer), "TEXT Total is %d. Enter a number between 1 and 9:\n", total);
        // send(client_socket, buffer, strlen(buffer), 0);

        // Send GO message
        send(client_socket, "GO\n", 3, 0);

        // Set up timeout
        timeout.tv_sec = TIMEOUT;
        timeout.tv_usec = 0;

        FD_ZERO(&readfds);
        FD_SET(client_socket, &readfds);

        // Wait for MOVE message or timeout
        int retval = select(client_socket + 1, &readfds, NULL, NULL, &timeout);

        if (retval == -1) {
            perror("select() failed");
            close(client_socket);
            client_sockets[index] = -1;
            current_clients--;
            return;
        } else if (retval == 0) {  // Timeout
            send(client_socket, "END\n", 4, 0);
            close(client_socket);
            client_sockets[index] = -1;
            current_clients--;
            printf("Player %d removed due to timeout.\n", index + 1);
            break;
        } else {
            // Receive client message
            memset(buffer, 0, sizeof(buffer));
            recv_len = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

            if (recv_len <= 0) {
                if (recv_len == 0) {
                    printf("Client %d disconnected\n", index + 1);
                } else {
                    perror("recv() failed");
                }
                close(client_socket);
                client_sockets[index] = -1;
                current_clients--;
                break;
            }

            buffer[recv_len] = '\0';

            // Check for client quitting
            if (strncmp(buffer, "QUIT", 4) == 0) {
                send(client_socket, "END\n", 4, 0);
                close(client_socket);
                client_sockets[index] = -1;
                current_clients--;
                printf("Player %d quit the game.\n", index + 1);
                break;
            }

            // Handle invalid input
            if (sscanf(buffer, "MOVE %d", &number) != 1 || number < 1 || number > 9) {
                error_count++;
                if (error_count >= MAX_ERRORS) {
                    send(client_socket, "Game over. \nEND\n", 18, 0);
                    close(client_socket);
                    client_sockets[index] = -1;
                    current_clients--;
                    printf("Player %d removed after %d invalid inputs.\n", index + 1, MAX_ERRORS);
                    break;
                }
                snprintf(buffer, sizeof(buffer), "TEXT ERROR Invalid input. Enter a number between 1 and 9 or 'quit':\n");
                send(client_socket, buffer, strlen(buffer), 0);
                continue;  // Go back to the start of the loop to re-prompt the user
            }

            // Valid move, update total
            total -= number;

            // Check for win condition
            if (total <= 0) {
                snprintf(buffer, sizeof(buffer), "TEXT You won!\nEND\n");
                send(client_socket, buffer, strlen(buffer), 0);
                close(client_socket);
                client_sockets[index] = -1;
                current_clients--;
                printf("Player %d won the game.\n", index + 1);
                break;
            } else {
                // Continue game
                snprintf(buffer, sizeof(buffer), "TEXT Total is %d. Enter a number between 1 and 9:\n", total);
                send(client_socket, buffer, strlen(buffer), 0);
            }
        }

        // Check if there's only one player left
        active_clients = 0;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != -1) {
                active_clients++;
            }
        }

        if (active_clients <= 1) {
            snprintf(buffer, sizeof(buffer), "Game over. Player %d won the game\n", index + 1);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] != -1) {
                    send(client_sockets[i], buffer, strlen(buffer), 0);
                    close(client_sockets[i]);
                    client_sockets[i] = -1;
                }
            }
            current_clients = 0;
            break;
        }
    }
}

void handle_signal(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        printf("\nShutting down the server...\n");
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != -1) {
                close(client_sockets[i]);
            }
        }
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <Port Number> <Game Type> <Game Arguments>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    char *game_type = argv[2];
    int num_players = atoi(argv[3]);

    if (strcmp(game_type, "numbers") != 0) {
        fprintf(stderr, "Error: Unsupported game type '%s'.\n", game_type);
        exit(EXIT_FAILURE);
    }

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind() failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("listen() failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Game Server started on port %d. Waiting for players...\n", port);

    // Initialize client sockets array
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = -1;
    }

    // Wait for players to join
    while (current_clients < num_players) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("accept() failed");
            continue;
        }

        printf("Player %d joined the game.\n", current_clients + 1);
        send(client_socket, "Welcome to the game\n: ", 23, 0);
        client_sockets[current_clients] = client_socket;
        current_clients++;
    }

    printf("All players have joined. Starting the game.\n");

    // Initialize total value
    total = 25;

    // Start game for each player in a child process
    for (int i = 0; i < num_players; i++) {
        if (fork() == 0) {
            close(server_socket);
            handle_client(client_sockets[i], i);
            exit(0);
        }
    }

    // Wait for all child processes to finish
    for (int i = 0; i < num_players; i++) {
        wait(NULL);
    }

    close(server_socket);
    return 0;
}
