#ifndef _HELPERS_H
#define _HELPERS_H 1

#include <stdio.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include "helpers.h"
#include <math.h>
#include <queue>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <forward_list>

/*
 * Macro de verificare a erorilor
 * Exemplu:
 *     int fd = open(file_name, O_RDONLY);
 *     DIE(fd == -1, "open failed");
 */

#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

#define BUFLEN		1552	// dimensiunea maxima a calupului de date
#define MAX_CLIENTS	100	// numarul maxim de clienti in asteptare

void sendOverTCP(int sockfd, char* buffer, int size);
void recvOverTCP(int sockfd, char* buffer);


#endif
