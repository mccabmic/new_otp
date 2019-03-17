#include "util.h"

int mod(int x, int y){
	return (x % y + y) % y;
}

bool validate(char* filename){
	size_t str_len = strlen(filename);
	int c;
	for (c = 0; c < str_len; c++){
		if (!isalpha(filename[c]) && !isspace(filename[c])){
			return false;
		}
	}
	return true;
}

char* read_file(char* filename){
	if (access(filename, F_OK) == -1){
		fprintf(stderr, "File does not exist: %s\n", filename);
		exit(2);
	}

	FILE *fp = fopen(filename, "r");
	if (fp < 0){
		fprintf(stderr, "Failure to open file %s\n", filename);
		exit(2);
	}

	size_t buffer_size;
	char* file_contents = NULL;
	size_t numCharacters = getdelim(&file_contents, &buffer_size, '\0', fp);
	fclose(fp);
	if (numCharacters != -1){
		file_contents[numCharacters-1]='\0';
		return file_contents;
	}
	else {
		free (file_contents);
		return 0;
	}
}

int sendall(int socket, char *buf, int *len){
	int total = 0;
	int bytesleft = *len;
	int n;

	while (total <* len){
		n = send(socket, buf+total, bytesleft, 0);
		if (n == -1){break;}
		total += n;
		bytesleft -= n;
	}
	*len = total;
	return n==-1?-1:0;
}

void my_encrypt(char* message, char* key){
	int c;
	
	for (c = 0; c < strlen(message); c++){
		if (message[c] == 32){
			message[c] = 91;
		}

		message[c] = (message[c] + key[c] - 65) % 27 + 65;
		if (message[c] == 91){
			message[c] = 32;
		} 
	} 
}

void my_decrypt(char* message, char* key){
	int c;
	for (c = 0; c < strlen(message); c++){
		
		if (message[c] == 32){
			message[c] = 91;
		}

		int t = message[c];
		t -= 65;
		t -= key[c];
		t = mod(t, 27);
		t += 65;
		
		message[c] = t;
		if (message[c] == 91){
			message[c] = 32;	
		}
	}
}
