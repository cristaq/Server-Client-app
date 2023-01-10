#include "helpers.h"

using namespace std;

void usage(char *file)
{
	fprintf(stderr, "Usage: %s ID server_address server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[])
{
	freopen("/dev/null", "w", stderr); // comenteaza pentru a vedea mesaje de eroare
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
	int sockfd, ret;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN];
	fd_set read_fds, tmp_fds;

	if (argc < 4) {
		usage(argv[0]);
	}

	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");

	// conectez clientul la server
	memset(buffer, 0, BUFLEN);
	strcpy(buffer, argv[1]);
	sendOverTCP(sockfd, buffer, strlen(buffer));
	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(sockfd, &read_fds);

	while (1) {
		tmp_fds = read_fds; 
		
		ret = select(sockfd + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");
		memset(buffer, 0, BUFLEN);

		if (FD_ISSET(STDIN_FILENO, &tmp_fds)) {
			// primim un mesaj de la tastatura
			
			fgets(buffer, BUFLEN, stdin);
			if (strncmp(buffer, "exit", 4) == 0) {
				// daca e "exit" anunt serverul si inchid clientul
				sendOverTCP(sockfd, buffer, strlen(buffer));
				close(sockfd);
				exit(0);
				

			} else if (strstr(buffer, "unsubscribe")) {
				// daca e unsubscribe verific sa am fix un cuvant dupa el si anunt serverul
				bool valid = true;
				
				if(buffer[12] == '\0' || buffer[12] == '\n') {
					fprintf(stderr, "Not enough arguments! Usage: unsubscribe topic\n");
					valid = false;
				} else {
					char* s = strchr(buffer + 13, ' ');
					if(s != NULL) {
						fprintf(stderr, "Too many arguments! Usage: unsubscribe topic\n");
						valid = false;
					}
				}

				if(valid) {
					sendOverTCP(sockfd, buffer, strlen(buffer));
					printf("Unsubscribed from topic.\n");
				}
				
			} else if (strstr(buffer, "subscribe")) {
				// daca e subscribe verific sa am fix 2 cuvinte dupa el si anunt serverul
				bool valid = true;
				char* s = strchr(buffer + 11, ' ');
				
				if(buffer[10] == '\0' || buffer[10] == '\n' || s == NULL || *(s + 1) == '\n') {
					fprintf(stderr, "Not enough arguments! Usage: subscribe topic SF\n");
					valid = false;
				} else {
					char* next = strchr(s + 1, ' ');
					if(next != NULL) {
						fprintf(stderr, "Too many arguments! Usage: subscribe topic SF\n");
						valid = false;
					}
				}
				
				if(valid) {
					sendOverTCP(sockfd, buffer, strlen(buffer));
					printf("Subscribed to topic.\n");
				}
				
			} else {
				fprintf(stderr, "Invalid command!\nCommands:\nsubscribe topic SF\nunsubscribe topic\nexit\n");
			}
		}


		if (FD_ISSET(sockfd, &tmp_fds)) {
			// primesc de la server un mesaj
			recvOverTCP(sockfd, buffer);
			
			if(strstr(buffer, "ID already used\n") != NULL) {
				// ID-ul e invalid
				fprintf(stderr, "ID already used\n");
				close(sockfd);
				return 0;
			}

			if (strncmp(buffer, "exit", 4) == 0) {
				// serverul se inchide
				close(sockfd);
				return 0;
			}

			// mesaj de la UDP primit de server
			printf("%s\n", buffer);
		}
	}

	close(sockfd);

	return 0;
}
