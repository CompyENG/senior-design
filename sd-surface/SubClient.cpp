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
#include "SubClient.hpp"

/*
SubClient::SubClient()
{
} */

bool SubClient::connectToSub(char * ip, int port) 
{
	sock_desc = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock_desc == -1)
    {
        printf("cannot create socket!\n");
        return false;
    }
    memset(&client, 0, sizeof(client));  
    client.sin_family = AF_INET;  
    client.sin_addr.s_addr = inet_addr(ip);  
    client.sin_port = htons(port);
    
    std::cout << "IP: " << ip << " PORT: " << port << std::endl;
    int count =0; //count the times we've tried to connect
    while(connect(sock_desc, (struct sockaddr*)&client, sizeof(client)) != 0 && count<5)
    {
		close(sock_desc);
        std::cout << "Cound not connect to server." << std::endl;
        sleep(5); //sleep for 5 seconds
		count++;
    }
    //we've tried to connect 5 times, but failed.
    if(count==5)
    {
		close(sock_desc);
        return false;
    } 
    return true;
}

bool SubClient::sendInt(int data)
{
	int k = 0;
	const char* pBytesOfData = (const char*)&data;
	int lengthOfBytes = sizeof(data);
	k = send(sock_desc, pBytesOfData, lengthOfBytes, 0);
	if (k == -1)
	{
		std::cout << "Cannot write to server!" << std::endl;
		return false;
	}
    return true;
}

