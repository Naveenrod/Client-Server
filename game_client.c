// Naveen Rodrigo s5286336
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_SIZE 1024

void handle_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void play_game(int sock) {
    char buffer[BUFFER_SIZE];
    int num;
    ssize_t recv_len;

    while (1) {
        // Clear buffer and receive data from server
        memset(buffer, 0, sizeof(buffer));
        recv_len = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (recv_len < 0) {
            handle_error("recv() failed");
        } else if (recv_len == 0) {
            printf("Server disconnected.\n");
            close(sock);
            return;
        }
        buffer[recv_len] = '\0';
        printf("%s", buffer);

        // Check for game over
        if (strncmp(buffer, "END", 3) == 0) {
            printf("Game over.\n");
            close(sock);
            return;
        }

        // Get user input
        memset(buffer, 0, sizeof(buffer));
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            handle_error("fgets() failed");
        }
        buffer[strcspn(buffer, "\n")] = 0;  // Remove newline character

        // Handle user input
        if (strcmp(buffer, "quit") == 0) {
            send(sock, "QUIT", 4, 0);
            printf("You quit the game.\n");
            close(sock);
            return;
        }

        // Convert input to integer and send MOVE command
        num = atoi(buffer);
        if (num < 1 || num > 9) {
            printf("Invalid input. Enter a number between 1 and 9 or 'quit'.\n");
            continue;
        }

        snprintf(buffer, sizeof(buffer), "MOVE %d", num);
        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            handle_error("send() failed");
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <Game Type> <Server Name> <Port Number>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *game_type = argv[1];
    char *server_name = argv[2];
    int port = atoi(argv[3]);

    if (strcmp(game_type, "numbers") != 0) {
        fprintf(stderr, "Error: Unsupported game type '%s'.\n", game_type);
        exit(EXIT_FAILURE);
    }

    int sock;
    struct sockaddr_in server_addr;
    struct hostent *host;

    if ((host = gethostbyname(server_name)) == NULL) {
        handle_error("Failed to resolve hostname");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr.s_addr, host->h_addr, host->h_length);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        handle_error("Socket creation failed");
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        handle_error("Connection failed");
    }

    play_game(sock);

    close(sock);
    return 0;
}
