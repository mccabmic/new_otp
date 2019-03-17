#ifndef UTIL_H
#define UTIL_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/wait.h>
#include <signal.h>

bool validate(char* filename);
char* read_file(char* filename);
int sendall(int socket, char *buf, int *len);
void my_encrypt(char* message, char* key);
void my_decrypt(char* message, char* key);

#endif // UTIL_H
