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

bool SubServer::listen(int port)
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

bool SubServer::send(uint32_t size_of_data, int8_t * data)
{

    int sent = 0;
    sent = ::send(temp_sock_desc, &size_of_data, 4, 0);
    while(sent < size_of_data) 
    {
        int bytes_sent = 0;
        bytes_sent = ::send(temp_sock_desc, &data+sent, size_of_data-sent, 0);
        if(bytes_sent == -1) {
            std::cout << "Cannot write to server!" << std::endl;
            return false;
        }
        sent += bytes_sent;
    }
    return true;
}

bool SubServer::recv(LVData * out) 
{
    int recvd = 0;
    uint32_t size;
    ::recv(temp_sock_desc, &size, 4, 0);
    while(recvd < size) 
    {
        int bytes_recvd;
        bytes_recvd = ::recv(temp_sock_desc, &out+recvd, size-recvd, 0);
        if(bytes_recvd == -1) {
            std::cout << "Cannot receive data!" << std::endl;
            return false;
        }
        recvd += bytes_recvd;
    }
    return true;
}

void SubClient::disconnect()
{
    close(sock_desc);
    close(temp_sock_desc);
}
