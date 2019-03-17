/*
 * server.c for TCP demo
 */
#define _GNU_SOURCE

#include "util.h"

#define MAXDATASIZE 4096
#define BACKLOG 5
#define FILESIZESTR 10

void sigchld_handler(int s){
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}

int main(int argc, char* argv[]){
	if (argc != 2){
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
	}
	
	char* PORT = argv[1];
	struct addrinfo hints, *servinfo, *traverse;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int rv;
	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}
	
	int listen_sock;
	int yes=1;
	for (traverse=servinfo; traverse != NULL; traverse=traverse->ai_next){
		if ((listen_sock = socket(traverse->ai_family, traverse->ai_socktype,
				traverse->ai_protocol)) == -1){
			fprintf(stderr, "%s: socket\n", __FILE__);
			continue;
		}
		
		if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR,&yes,
				sizeof(int)) == -1){
			fprintf(stderr, "%s: setsockopt\n", __FILE__);
			exit(1);
		}
		
		if (bind(listen_sock, traverse->ai_addr, traverse->ai_addrlen) == -1){
			close(listen_sock);
			fprintf(stderr, "%s: bind\n", __FILE__);
			continue;
		}
		break;
	}

	freeaddrinfo(servinfo);
	if (traverse == NULL){
		fprintf(stderr, "%s: failed to bind\n", __FILE__);
		exit(1);
	}

	if (listen(listen_sock, BACKLOG) == -1){
		fprintf(stderr, "%s: listen error\n", __FILE__);
		exit(1);
	}

	// Reap zombies
	struct sigaction sa;
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1){
		fprintf(stderr, "%s: sigaction error\n", __FILE__);
		exit(1);
	}

	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	char s[INET6_ADDRSTRLEN];
	int task_sock;
	
	while(1){
		sin_size = sizeof(their_addr);
		task_sock = accept(listen_sock, (struct sockaddr*) &their_addr, &sin_size);
		if (task_sock == -1){
			fprintf(stderr, "%s: action socket error\n", __FILE__);
			continue;
		}
		
		inet_ntop(their_addr.ss_family, &their_addr, s, sizeof(s));
		if (!fork()){ // Child
			close(listen_sock);
			char id[1];
			
			// If not authorized, close connection
			recv(task_sock, id, sizeof(id), 0);
			if( id[0] != 'e'){
				send(task_sock, "0", 1, 0);
				close(task_sock);
				exit(2);
			}

			else{
				send(task_sock, "1", 1, 0);
			}
			
			// Receive string length
			char size[20];
			recv(task_sock, size, FILESIZESTR, 0);
			int str_size = atoi(size);

			char* message = malloc(sizeof(char) * str_size);
			char buffer[MAXDATASIZE] = {0};
			int nbytes = 0;
			int bytesSum = 0;

			while((nbytes = recv(task_sock, buffer, str_size, 0)) > 0){
				bytesSum += nbytes;
				strcat(message, buffer);
				if (bytesSum >= sizeof(char) * str_size){break;}
				memset(buffer, '\0', sizeof(char) * MAXDATASIZE - 1);
			}

			message[str_size] = '\0';
			
			nbytes = 0;
			bytesSum = 0;
			char* key = malloc(sizeof(char) * str_size);
			memset(buffer, '\0', sizeof(char) * MAXDATASIZE - 1);
			memset(key, '\0', str_size);
			while((nbytes = recv(task_sock, buffer, str_size, 0)) > 0){
				bytesSum += nbytes;
				strcat(key, buffer);
				if (bytesSum >= sizeof(char) * str_size){break;}
				memset(buffer, '\0', sizeof(char) * MAXDATASIZE - 1);
			}
			
			key[str_size] = '\0';
			my_encrypt(message, key);
			printf("%s\n", message);
			sendall(task_sock, message, &str_size);
			
			close(task_sock);
			free(key);
			free(message);
			
			exit(0);
		}
		// Parent
		close(task_sock);
	} // Main Loop
	return 0;
}
