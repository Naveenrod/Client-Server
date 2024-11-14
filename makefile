# Makefile for Game Server and Client

CC = gcc
CFLAGS = -Wall -Wextra -O2

# Targets
all: game_server game_client

game_server: game_server.c
	$(CC) $(CFLAGS) -o game_server game_server.c

game_client: game_client.c
	$(CC) $(CFLAGS) -o game_client game_client.c

clean:
	rm -f game_server game_client

.PHONY: all clean
