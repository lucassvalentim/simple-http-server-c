all: client/main server/main

client/main: client/main.c
	gcc client/main.c -o client/main

server/main: server/main.c
	gcc server/main.c -o server/main

clean:
	rm server/main
	rm client/main