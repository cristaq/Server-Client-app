#include "helpers.h"

using namespace std;

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}

//structura cu toate informatiile relevante pentru un abonat/client
struct Subscriber {
	string ID;
	bool connected;
	queue<string> standby;
	unordered_map<string, bool> SF;
	int socket;
};

unordered_map<string, Subscriber*> subscribers; //mapeaza ID la subscribers
unordered_map<string, forward_list<Subscriber*>> topicToSub; //mapeaza topic la o lista de subscribers
unordered_map<int, Subscriber*> sockets; //mapeaza sockets la subscribers

int main(int argc, char *argv[])
{
	freopen("/dev/null", "w", stderr); // comenteaza pentru a vedea mesaje de eroare
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
	int sockfdTCP, sockfdUDP, portno, newsockfd;
	char buffer[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr;
	int i, ret;
	socklen_t clilen;

	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax;			// valoare maxima fd din multimea read_fds

	if (argc < 2) {
		usage(argv[0]);
	}

	// se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	sockfdTCP = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfdTCP < 0, "socket");

	sockfdUDP = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(sockfdUDP < 0, "socket");

	portno = atoi(argv[1]);
	DIE(portno == 0, "atoi");

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(sockfdTCP, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind_TCP");

	ret = bind(sockfdUDP, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind_UDP");

	ret = listen(sockfdTCP, MAX_CLIENTS);
	DIE(ret < 0, "listen");

	// se adauga file descriptorii in multimea read_fds
	FD_SET(sockfdTCP, &read_fds);
	FD_SET(sockfdUDP, &read_fds);
	FD_SET(STDIN_FILENO, &read_fds);
	fdmax = max(sockfdTCP, sockfdUDP);

	// oprim algoritmul lui Nagle
	int yes = 1;
	int result = setsockopt(sockfdTCP, IPPROTO_TCP, TCP_NODELAY, (char*) &yes, sizeof(int));
	DIE(result < 0, "Nagle");

	while (1) {
	
		tmp_fds = read_fds; 
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		for (i = 0; i <= fdmax; i++) {

			if (FD_ISSET(i, &tmp_fds)) {
				// verific fiecare file descriptor si vad daca e activ

				if(i == STDIN_FILENO) {
					// primim un mesaj de la tastatura
					memset(buffer, 0, BUFLEN);
					fgets(buffer, BUFLEN - 1, stdin);
					if (strncmp(buffer, "exit", 4) == 0) {
						for(auto i : sockets) {
							// anunt toti clientii activi de inchiderea serverului
							if(i.second->connected) {
								sendOverTCP(i.first, buffer, strlen(buffer));
							}
							close(i.first);
						}
						int enable = 1;
						
						close(sockfdTCP);
						close(sockfdUDP);
						exit(0);
					} else {
						fprintf(stderr, "Unknown command\n");
					}
					
				} else if (i == sockfdTCP) {
					// cerere pe socketul de listen
					// avem un nou client TCP
					clilen = sizeof(cli_addr);
					newsockfd = accept(sockfdTCP, (struct sockaddr *) &cli_addr, &clilen);
					DIE(newsockfd < 0, "accept");

					memset(buffer, 0, BUFLEN);
					recvOverTCP(newsockfd, buffer);

					if(subscribers.count(buffer) && subscribers.at(buffer)->connected) {
						// cazul in care un client incearca sa se conecteze cu un ID deja in folosinta
						// refuz conexiunea si anunt clientul
						printf("Client %s already connected.\n", buffer);
						memset(buffer, 0, BUFLEN);
						strcpy(buffer, "ID already used\n");
						sendOverTCP(newsockfd, buffer, strlen(buffer));
						close(newsockfd);

					} else if(!subscribers.count(buffer)) {
						// avem un client nou valid
						// se adauga noul socket intors de accept() la multimea descriptorilor de citire
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax) { 
							fdmax = newsockfd;
						}

						// initializam structura si o adaugam in mapurile relevante
						Subscriber* s = new Subscriber;
						s->ID = buffer;
						s->connected = true;
						s->socket = newsockfd;
						sockets.insert({newsockfd, s});
						subscribers.insert({buffer, s});
						printf("New client %s connected from %s:%d\n", buffer,
								inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

						
					} else {
						// client care revine
						// ii actualizez informatiile
						Subscriber* s = subscribers.at(buffer);
						s->connected = true;
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax) { 
							fdmax = newsockfd;
						}

						s->socket = newsockfd;
						sockets.insert({newsockfd, s});
						printf("New client %s connected from %s:%d\n", buffer,
								inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

						// clientul care revine primeste toate mesajele salvate in coada
						while(!s->standby.empty()) {
							string mssg = s->standby.front();
							memset(buffer, 0, BUFLEN);
							strcpy(buffer, mssg.c_str());
							sendOverTCP(newsockfd, buffer, strlen(buffer));
							s->standby.pop();
						}
						
					}

				} else if(i == sockfdUDP) {
					// avem un mesaj UDP
					clilen = sizeof(cli_addr);
					memset(buffer, 0, BUFLEN);
					recvfrom(sockfdUDP, buffer, BUFLEN, 0, (struct sockaddr*) &serv_addr, &clilen);
					
					// topicul sunt primii 50 de bytes din mesaj
					// type e byte-ul 51
					char topic[51];
					memcpy(topic, buffer, 50);
					topic[50] = '\0';
					char type = buffer[50];
					char message[1700];
					
					// mesajele sunt formatate dupa type
					switch (type)
					{
						case 0: {
							char sign = buffer[51];
							int number;
							memcpy(&number, buffer + 52, 4);
							number = ntohl(number);

							if(sign == 1) {
								number *= (-1);
							}
							sprintf(message, "%s:%d - %s - INT - %d", inet_ntoa(serv_addr.sin_addr), serv_addr.sin_port, topic, number);
							break;
						}
						
						case 1: {
							uint16_t number;
							memcpy(&number, buffer + 51, sizeof(uint16_t));
							number = ntohs(number);
							float num = number / 100.0;
							sprintf(message, "%s:%d - %s - SHORT_REAL - %.*f", inet_ntoa(serv_addr.sin_addr), serv_addr.sin_port, topic, 2, num);
							break;
						}

						case 2: {
							char sign = buffer[51];
							int number;
							memcpy(&number, buffer + 52, 4);
							number = ntohl(number);
							char power = buffer[56];
							float newNumber = ((float) number) * (pow(10, (-1) * power));
							if(sign == 1) {
								newNumber *= (-1);
							}
							sprintf(message, "%s:%d - %s - FLOAT - %.*f", inet_ntoa(serv_addr.sin_addr), serv_addr.sin_port, topic, power, newNumber);
							break;
						}

						case 3: {
							sprintf(message, "%s:%d - %s - STRING - %s", inet_ntoa(serv_addr.sin_addr), serv_addr.sin_port, topic, buffer + 51);
							break;
						}

						default:
							fprintf(stderr, "Invalid Type");
							break;
					}

					// trimit mesajul la toti clientii abonati daca sunt conectati
					// daca sunt deconectati si au SF 1 pentru acest topic, il pun in coada clientului
					for(auto i : topicToSub[topic]) {
						if(i->connected) {
							sendOverTCP(i->socket, message, strlen(message));
						} else {
							if(i->SF.at(topic)) {
								i->standby.push(message);
							}
						}
					}

				} else {
					// primim un mesaj de la un client TCP deja conectat
					recvOverTCP(i, buffer);
					Subscriber* s = sockets.at(i);

					if(strncmp(buffer, "exit", 4) == 0) {
						// daca mesajul este "exit", deconectez clientul
						s->connected = false;
						cout << "Client " << s->ID << " disconnected.\n";
						sockets.erase(s->socket);
						FD_CLR(i, &read_fds);
						close(i);

					} else if(strncmp(buffer, "unsubscribe", 11) == 0) {
						// daca mesajul este "unsubscribe" il elemin din map
						strtok(buffer, " ");
						char* topic = strtok(NULL, " ");
						topic[strlen(topic) - 1] = '\0';
						topicToSub[topic].remove(s);

					} else if(strncmp(buffer, "subscribe", 9) == 0){
						// daca mesajul este "subscribe" il adaug in map
						strtok(buffer, " ");
						char* topic = strtok(NULL, " ");
						char* sf = strtok(NULL, " ");
						if(!count(topicToSub[topic].begin(), topicToSub[topic].end(), s)) {
							topicToSub[topic].push_front(s);
						}
						s->SF[topic] = atoi(sf);
					}
					
				}
			}
		}
	}

	return 0;
}
