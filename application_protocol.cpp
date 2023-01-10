#include "helpers.h"


// send si receive implementat astfel incat sa incadram bine mesajele

void sendOverTCP(int sockfd, char* buffer, int size) {
	// anunt destinatarul de diminesiunea mesajului
	int bytes_sent = send(sockfd, &size, sizeof(int), 0);
	DIE(bytes_sent < 0, "No size");

	int bytes_remaining = size;

	// trimit mesajul pana cand s-a trimis tot
	do {
		bytes_sent = send(sockfd, buffer + (size - bytes_remaining), bytes_remaining, 0);
		DIE(bytes_sent < 0, "Couldn't send to server!");
		bytes_remaining -= bytes_sent;
	} while (bytes_remaining > 0);
	
}

void recvOverTCP(int sockfd, char* buffer) {
	// primesc prima oara dimensiunea mesajului care vine
	memset(buffer, 0, BUFLEN);
	int bytes_recv = recv(sockfd, buffer, sizeof(int), 0);
	DIE(bytes_recv < 0, "No connection");
	int size;
	memcpy(&size, buffer, sizeof(int));

	memset(buffer, 0, BUFLEN);
	int bytes_remaining = size;
	// primesc mesajul pana cand am primit tot
	do {
		bytes_recv = recv(sockfd, buffer + (size - bytes_remaining), bytes_remaining, 0);
		DIE(bytes_recv < 0, "Couldn't send to server!");
		bytes_remaining -= bytes_recv;
	} while (bytes_remaining > 0);
}