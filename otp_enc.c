/*
 * client.c
 */
#define _GNU_SOURCE

#include "util.h"
#define FILESIZESTR 10
#define MAXDATASIZE 1000

int main(int argc, char *argv[]){
	// verify arguments
	if (argc != 4){
		printf("%s: usage - %s plaintext key port\n", __FILE__, __FILE__);
		exit(1);
	}
	
	// set port
	char* PORT = argv[3];
	
	// read contents of plaintext and verify no bad chars
	char* plaintext = read_file(argv[1]);
	char* key = read_file(argv[2]);
	
	int pt_len, key_len;
	pt_len = strlen(plaintext);
	key_len = strlen(key);
	if (pt_len != key_len){
		fprintf(stderr, "%s: plaintext len != key len\n", __FILE__);
		exit(1);
	}
	
	if (!validate(plaintext)){
		fprintf(stderr, "%s: input contains bad characters\n", __FILE__);
		exit(1);
	};

	if (!validate(key)){
		fprintf(stderr, "%s: input contains bad characters\n", __FILE__);
		exit(1);
	}
		
	struct addrinfo hints, *servinfo, *traverse;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // change to AF_UNSPEC for IPv6 comp
	hints.ai_socktype = SOCK_STREAM;
	int rv;

	if ((rv = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0){
		fprintf(stderr, "%s: getaddrinfo error: %s\n", __FILE__, gai_strerror(rv));
		return 2;
	}

	int connect_socket;
	char s[INET6_ADDRSTRLEN];

	for (traverse = servinfo; traverse != NULL; traverse=traverse->ai_next){
		if ((connect_socket = socket(traverse->ai_family, traverse->ai_socktype,
				traverse->ai_protocol)) == -1){
			fprintf(stderr, "%s: socket error\n", __FILE__);
			continue;
		}
		
		inet_ntop(traverse->ai_family, traverse->ai_addr, s, sizeof(s));
		if (connect(connect_socket, traverse->ai_addr, traverse->ai_addrlen) == -1){
			close(connect_socket);
			fprintf(stderr, "%s: connect error\n", __FILE__);
			continue;
		}
		break;
	}

	if (traverse == NULL){
		fprintf(stderr, "%s: could not connect\n", __FILE__);
		free(plaintext);
		free(key);
		exit(2);
	}

	freeaddrinfo(servinfo);
	
	// verify identity to server
	send(connect_socket, "e", 1, 0);
	// receive confirmation from server
	char auth = '\0';
	recv(connect_socket, &auth, 1, 0);

	// if not authorized, exit and print error
	if (auth != '1'){
		fprintf(stderr, "%s: server refused connection\n", __FILE__);
		close(connect_socket);
		free(plaintext);
		free(key);
		exit(2);	
	}
	// otherwise send the size of the data
	else{
		char filesize[10];
		sprintf(filesize, "%d", (int)strlen(plaintext));
		send(connect_socket, filesize, sizeof(filesize), 0);
	}
	
	int size = strlen(plaintext);
	
	sendall(connect_socket, plaintext, &size);
	sendall(connect_socket, key, &size);

	free(key);
	free(plaintext);
	
	char* response = malloc(sizeof(char) * size);
	memset(response, '\0', size);
	char buffer[MAXDATASIZE] = {0};
	int nbytes = 0;
	while((nbytes = recv(connect_socket, buffer, size, 0)) > 0){
		strcat(response, buffer);
		memset(buffer, '\0', sizeof(char) * MAXDATASIZE - 1);
	}
	
	close(connect_socket);
	response[size] = '\0';		
	printf("%s\n", response);
	return 0;
}
