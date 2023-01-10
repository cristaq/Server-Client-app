Cristache Cristian-Valentin

To receive error messages in case of wrong input
(this is displayed in the stderr) the first line of the main menu is commented out: 
freopen("/dev/null", "w", stderr). Any message that is not in
requirement is disabled by default by this function!!

I'll start by presenting the application level protocol
since all server-client communication takes place through it.

application_protocol.cpp:
    Here are implemented 2 functions that aim to resolve
the message framing problem caused by TCP. We consider
that messages can be truncated (if they are too long) or concatenated
(if they are too short).

    To handle this situation, the sender always sends first
an INT (4 bytes) specifying the length of the message to be sent.
Having only 4 bytes guarantees that TCP will not truncate it, nor will it be
concatenated because the receiver receives only 4 bytes first.

    The sender sends the message in a do while until it is all the message
sent. The receiver does the same with receiving. We use the
returned by the send and receive functions that tell us how much we receive and how much
we actually send.

    The solution is efficient because you only do one more send and one more receive.    
and the messages are of varying sizes (I don't send whatever 2000 bytes 
even if I only want to send 4 bytes).

server.cpp:
    Most of the information about a client is in the Subscrieber structure.
Any other relevant information is mapped into 3 unorder_maps. Activity
server's activity consists of constantly checking all descriptor files and
act on their activation.
    If fd = STDIN_FILENO, we only have one relevant keyboard command,
exit. When we get exit we notify all clients, release the sockets and give exit(0).
Any other keyboard command is invalid.
    If fd = sockfdUDP we know we have a UDP message. This must be parsed after
instructions in the request (see comments in the code).
    If fd = sockfdTCP we know we have a new connection request. We accept
or not the client depending on the ID and if it is already in use.
    If fd = other we have a message from an already connected client, which can
be: exit, subscribe, unsubscribe.

subscriber.cpp:
    In a similar way the client works, just with fewer cases.
It can receive keyboard messages like exit, subscribe and unsubscribe,
but also from the server: exit, UDP messages, connection refused. It is addressed
each case individually and explained in the code comments.
