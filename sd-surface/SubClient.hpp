#ifndef SUBCLIENT
#define SUBCLIENT

#include "SDL/SDL.h"
#include <stdint.h>
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>

class SubClient
{
	public:
	struct sockaddr_in client;
	int sock_desc;
	bool connectToSub(char * ip, int port);
	bool sendInt(int data);
	bool sendCommands(int8_t * data);
	void disconnectFromSub();
};

#endif /* SUBCLIENT */
