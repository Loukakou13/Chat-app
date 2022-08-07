#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <err.h>
#include <pthread.h>

#define LENGTH 2048
char name[32];
int flag = 0;
int socketServer;

void str_trim_lf(char* str, int length){
	for(int i = 0; i < length; i++){
		if(str[i] == '\n'){
			str[i] = '\0';
			break;
		}
	}
}

void *send_msg_handler(){
	char msg[LENGTH];
	char buffer[LENGTH + 40];
	while(1){
		fgets(msg, LENGTH, stdin);
		str_trim_lf(msg, LENGTH);

		if(strcmp(msg, "exit") == 0){
			flag = 1;
			break;
		}
		else if (strlen(msg) != 0){
			sprintf(buffer, "%s: %s\n", name, msg);
			send(socketServer, buffer, strlen(buffer), 0);
		}
		bzero(msg, LENGTH);
		bzero(buffer, LENGTH + 32);
	}
	pthread_exit(NULL);
}

void *recv_msg_handler(){
	char msg[LENGTH];
	int c;
	int recvState;
	while(1){
		recvState = recv(socketServer, msg, LENGTH, 0);
		if(recvState > 0){
			printf("CA VA MARCHER ?\n");
			printf("\33[2k\r");
//			fflush(stdin);
			while((c = getchar()) != '\n' && c != EOF){};
			printf("%s", msg);
			bzero(&msg, sizeof(msg));
		}
		else{
			flag = 1;
			break;
		}
	}
	pthread_exit(NULL);
}

int main(){
	
	// Socket	
	socketServer = socket(AF_INET, SOCK_STREAM, 0);
	
	// Address
	struct sockaddr_in addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = inet_addr("127.0.0.7");
	addrServer.sin_port = htons(19840);

	// Connect
	int connection_status = connect(socketServer, (struct sockaddr *) &addrServer, sizeof(addrServer));

	// check for error with the connection
	if (connection_status != 0){
		printf("failed to connect\n");
		return 1;
	}
	printf("connected\n");	


	// Send name
	char serverResponse[256];
	recv(socketServer, &serverResponse, sizeof(serverResponse), 0);
	printf("%s", serverResponse);
	fgets(name, 32, stdin);
	str_trim_lf(name, strlen(name));
	send(socketServer, name, sizeof(name), 0);

	printf("=== WELCOME TO THE CHATROOM ===\n");


	pthread_t send_msg_thread;
	if(pthread_create(&send_msg_thread, NULL, send_msg_handler, NULL) != 0){
		errx(EXIT_FAILURE, "ERROR: send_msg_thread");
	}

	pthread_t recv_msg_thread;
	if(pthread_create(&recv_msg_thread, NULL, recv_msg_handler, NULL) != 0){
		errx(EXIT_FAILURE, "ERROR: recv_msg_thread");
	}


	while(1){
		if(flag){
			printf("Good bye !\n");
			break;
		}
	}
	

	// close the socket
	close(socketServer);

	return 0;
}
