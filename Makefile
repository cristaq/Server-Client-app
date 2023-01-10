# Protocoale de comunicatii:
# Laborator 8: Multiplexare
# Makefile

CFLAGS = -Wall -g

# Portul pe care asculta serverul (de completat)
PORT = 12343

# Adresa IP a serverului (de completat)
IP_SERVER = 127.0.0.1

build: server subscriber

# Compileaza server.c
server: server.cpp
		g++ server.cpp application_protocol.cpp -o server -g

# Compileaza client.c
subscriber: subscriber.cpp
			g++ subscriber.cpp application_protocol.cpp -o subscriber -g

.PHONY: clean

clean:
	rm -f server subscriber
