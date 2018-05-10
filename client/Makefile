all: clean client

default: all

client:
	gcc -o client client.c url/url.c utils/utils.c http/http.c -Wall -Werror -g -lm

clean:
	rm -rf client
