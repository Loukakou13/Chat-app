#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <err.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 2048
#define NAME_LEN 32


// Client Strucutre
typedef struct{
	struct sockaddr_in address;
	int socket;
	int uid;
	char name[NAME_LEN];
} client_t;

client_t *clients[MAX_CLIENTS];
int usersOnline = 0;



char* str_ip_addr(char *str, struct sockaddr_in addr){
	sprintf(str, "%d.%d.%d.%d", addr.sin_addr.s_addr & 0xff,
	(addr.sin_addr.s_addr & 0xff00) >> 8,
	(addr.sin_addr.s_addr & 0xff0000) >> 16,
	(addr.sin_addr.s_addr & 0xff000000) >> 24);
	return str;
}

void send_msg(char *msg, int uid){
	for(int i = 0; i < MAX_CLIENTS; i++){
		if(clients[i] && clients[i]->uid != uid){
			write(clients[i]->socket, msg, strlen(msg)); 
		}
	}

}

void queue_add(client_t *client){
	for(int i = 0; i < MAX_CLIENTS; i++){
		if(!clients[i]){
			clients[i] = client;
			break;
		}
	}
}

void queue_remove(int uid){
	for(int i = 0; i < MAX_CLIENTS; i++){
		if(clients[i]->uid == uid){
			clients[i] = NULL;
			break;
		}
	}
}

void* client_handler(void *arg){
	usersOnline++;
	client_t *client = (client_t *)arg;
	// Client name
	char name[32];
	send(client->socket, "Enter your name: ", sizeof("Enter your name: "), 0);
	recv(client->socket, &name, sizeof(name), 0);
	strcpy(client->name, name);
	char strIP[16];
	printf("%s(%s)(%d) join the chatroom.\n", client->name, str_ip_addr(strIP, client->address), client->socket);

	char buffer[BUFFER_SIZE];
	while(1){
		int recvState = recv(client->socket, buffer, BUFFER_SIZE, 0);
		if(recvState > 0){
			//printf("%s", buffer);
			send_msg(buffer, client->uid);
		}
		else{
			break;
		}
	}

	sprintf(buffer, "%s has left.\n", client->name);
	printf("%s",buffer);
	send_msg(buffer, client->uid);
	// Close client socket
	queue_remove(client->uid);
	close(client->socket);
	free(client);
	usersOnline--;

	pthread_exit(NULL);
}

int main(){
	// Socket
	int socketServer;
	socketServer = socket(AF_INET, SOCK_STREAM, 0);

	// Address
	struct sockaddr_in addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = inet_addr("127.0.0.7");
	addrServer.sin_port = htons(19840);

	// Bind
	if(bind(socketServer, (const struct sockaddr *) &addrServer, sizeof(addrServer)) < 0){
		errx(EXIT_FAILURE, "ERROR: bind\n");
	}
	printf("bind : %d\n", socketServer);

	// Listen
	if(listen(socketServer, MAX_CLIENTS) < 0){
		errx(EXIT_FAILURE, "ERROR: listen\n");
	}
	printf("listenning...\n");


	pthread_t thread;
	printf("=== CHATROOM INITIATED ===\n");
	int uid = 0;

	while(1){
		struct sockaddr_in addrClient;
		socklen_t csize = sizeof(addrClient);
		int socketClient = accept(socketServer, (struct sockaddr *) &addrClient, &csize);

		if(usersOnline + 1 == MAX_CLIENTS){
			printf("Connection Rejected: Maximum clients connected");
			close(socketClient);
			continue;
		}

		printf("new connection accepted\n");

		client_t *client = malloc(sizeof(client_t));
		client->address = addrClient;
		client->socket = socketClient;
		client->uid = uid++;


		queue_add(client);
		pthread_create(&thread, NULL, client_handler, client);
		
		// Reduce CPU usage
		sleep(0.5);
	}

	// close the socket
	close(socketServer);

	return 0;
}
