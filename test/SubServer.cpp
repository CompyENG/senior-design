//The headers
#include "SDL/SDL.h"
#include <iostream>
#include <stdint.h>
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <stdio.h>  
#include <string.h>  
#include <stdlib.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include "SubServer.hpp"

/*
SubClient::Subserver()
{
} */

bool SubServer::listenForClient(int port)
{
	sock_desc = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock_desc == -1)
    {
        printf("cannot create socket!\n");
        return false;
    }
    memset(&server, 0, sizeof(server));  
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;  
    server.sin_port = htons(50000);  
    if (bind(sock_desc, (struct sockaddr*)&server, sizeof(server)) != 0)
    {
        std::cout << "cannot bind socket!\n" << std::endl;
        close(sock_desc);  
        return false;
    }
    
    if (listen(sock_desc, 20) != 0)
    {
        std::cout << "cannot listen on socket!\n" << std::endl;
        close(sock_desc);  
        return false;
    }
    
    memset(&client, 0, sizeof(client));  
    socklen_t len = sizeof(client); 
    temp_sock_desc = accept(sock_desc, (struct sockaddr*)&client, &len);  
    if (temp_sock_desc == -1)
    {
        std::cout << "cannot accept client!\n" << std::endl;
        close(sock_desc);  
        return 0;
    }
    
    std::cout << "Server on PORT: " << port << std::endl;
    return true;
}

int SubServer::receiveInt()
{
	int k;  
    int integer;
	k = recv(temp_sock_desc, &integer, 4, 0);
	
	if (!recv)
	{
		std::cout << "\ncannot read from client!\n" << std::endl;
		return -1;
	}
	else if (recv == 0)
	{
		std::cout << "\nclient disconnected.\n" << std::endl;
		return 0;
	}
	else 
	{
		return integer;
	}
	
}

int8_t * SubServer::receiveCommands()
{
	int k;  
    int8_t * commands[7];
    bzero(commands, 7);
	k = recv(temp_sock_desc, &commands, sizeof(commands), 0);
	if (!recv)
	{
		std::cout << "cannot read from client!" << std::endl;
	}
	else if (recv == 0)
	{
		std::cout << "client disconnected." << std::endl;
	}
	else 
	{
		std::cout << "commands[FORWARD] = " << commands[0] << std::endl;
		std::cout << "commands[LEFT] = " <<  commands[1] << std::endl;
		std::cout << "commands[PITCH] = " <<  commands[2] << std::endl;
		std::cout << "commands[ZOOM] = " <<  commands[3] << std::endl;
		std::cout << "commands[ASCEND] = " <<  commands[4] << std::endl;
		std::cout << "commands[SHOOT] = " <<  commands[5] << std::endl;
		std::cout << "commands[LIGHTS] = " << commands[6] << std::endl;
		return commands;
	}
}

void SubServer::disconnectFromClient()
{
	close(sock_desc);
	close(temp_sock_desc);
}
