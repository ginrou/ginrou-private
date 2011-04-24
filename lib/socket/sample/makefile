target : client server

client : client.c
	gcc -o client client.c ipc_sub3.c

server : server.c
	gcc -o server server.c ipc_sub3.c

clean :
	rm server client *~
