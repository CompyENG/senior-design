#ifndef SUBSERVER
#define SUBSERVER

#include "SDL/SDL.h"
#include <stdint.h>
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>

class SubServer
{
	public:
	struct sockaddr_in server;
	struct sockaddr_in client;
	int temp_sock_desc;
	int sock_desc;
	bool listenForClient(int port);
	int receiveInt();
	int8_t * receiveCommands();
	void disconnectFromClient();
};

#endif /* SUBCLIENT */
