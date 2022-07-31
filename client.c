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

#define NAME_LEN 32

void str_trim_lf(char* str, int length){
	for(int i = 0; i < length; i++){
		if(str[i] == '\n'){
			str[i] = '\0';
			break;
		}
	}
}

void send_msg_handler(char *msg){
	
}

void recieve_msg_handler(){

}

int main(){
	
	// create a socket	
	int socketServer;
	socketServer = socket(AF_INET, SOCK_STREAM, 0);
	
	// specify an address for the socket
	struct sockaddr_in addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = inet_addr("127.0.0.7");
	addrServer.sin_port = htons(19840);

	// try to connect to the server
	int connection_status = connect(socketServer, (struct sockaddr *) &addrServer, sizeof(addrServer));

	// check for error with the connection
	if (connection_status != 0){
		printf("failed to connect\n");
		return 1;
	}
	printf("connected\n");	


	// Send name
	char name[NAME_LEN];
	char serverResponse[256];
	recv(socketServer, &serverResponse, sizeof(serverResponse), 0);
	printf("%s", serverResponse);
	fgets(name, NAME_LEN, stdin);
	str_trim_lf(name, strlen(name));
	send(socketServer, name, sizeof(name), 0);


	// print out the server's response
//	printf("%s", serverResponse);

	// close the socket
	close(socketServer);

	return 0;
}
