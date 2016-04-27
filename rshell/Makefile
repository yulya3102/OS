all: rshell client

rshell: rshell.c common.h common.c daemon.h daemon.c
	gcc -Wall -Wextra -o rshell rshell.c common.c daemon.c -lutil

client: client.c common.h common.c
	gcc -Wall -Wextra -o client client.c common.c
