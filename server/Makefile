all: clean server

default: all

server:
	gcc -o server server.c http/http.c utils/utils.c -Wall -Werror -g -pthread

clean:
	rm -rf server
