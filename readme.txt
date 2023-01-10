Cristache Cristian-Valentin 321CD
(Implementarea are ca baza elemente de cod din laboratorul 7 si 8)

!!Pentru a primi mesaje de eroare in caz de input gresit
(acesta e afisat la stderr) se comnteaza prima linie din main: 
freopen("/dev/null", "w", stderr). Orice mesaj care nu e in
cerinta este dezactivat implicit de aceasta functie!!

Voi incepe prin prezentarea protocolului de nivel aplicatie
intrucat toata comunicarea server-client are loc prin acesta.

aplication_protocol.cpp:
    Aici sunt implementate 2 functii care isi propun sa rezolve
problema de incadrare a mesajelor provocata de TCP. Avem in vedere
ca mesajele se pot trunchia (daca sunt prea lungi) sau se pot concatena
(daca sunt prea scurte).

    Pentru a gestiona situatia, senderul trimite mereu prima oara
un INT (4 bytes) in care sepcifica lungimea mesajului ce va fi timis.
Avand doar 4 bytes avem garantia ca TCP nu-l va trunchia, si nici nu va fi
concatenat deoarece receiverul primeste prima oara doar 4 bytes.

    Senderul trimite mesajul intr-un do while pana cand e tot mesajul
trimis. Receiverul face la fel cu primirea. Ne folosim de valoarea
intoarsa de functiile send si receive care ne spun cat primim si cat
trimitem cu adevarat.

    Solutia este eficienta deoarece se face doar un send si un receive in plus    
si mesajele sunt de dimensiuni variate (nu trimit orice ar fi 2000 de bytes 
chiar daca vreau sa trimit doar 4 bytes).

server.cpp:
    Majoritatea informatiilor despre un client sunt in structura Subscrieber.
Orice alta informatie relevanta este mapata in 3 unorder_maps. Activitatea
serverului se rezuma la a verifica constant toti file descriptorii si
sa actioneze in functie de activarea lor.
    Daca fd = STDIN_FILENO, avem doar o comanda relevanta de la tastatura,
exit. Cand primim exit anuntam toti clientii, eliberam socketii si dam exit(0).
Orice alta comanda de la tastatura este invalida.
    Daca fd = sockfdUDP stim ca avem un mesaj UDP. Acesta trebuie parsat dupa
instructiunile din cerinta (a se vedea comentarii in cod).
    Daca fd = sockfdTCP stim ca avem un cerere de conexiune noua. Aceptam
sau nu clientul in functie de ID si daca acesta e deja in folosinta.
    Daca fd = altceva avem un mesaj de la un client deja conectat, care poate
fi: exit, subscribe, unsubscribe.

subscriber.cpp:
    Intr-o maniera similara merge si clientul, doar cu mai putine cazuri.
El poate primi mesaje de la tastatura, de genul exit, subscribe si unsubscribe,
dar si de la server: exit, mesajele UDP, refuzul conexiunii. Este abordat
fiecare caz in parte si explicat in comentariile din cod.