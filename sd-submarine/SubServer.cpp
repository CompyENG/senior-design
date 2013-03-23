//The headers
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
#include <libptp++.hpp>

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
    if (::bind(sock_desc, (struct sockaddr*)&server, sizeof(server)) != 0)
    {
        std::cout << "cannot bind socket!\n" << std::endl;
        close(sock_desc);  
        return false;
    }
    
    if (::listen(sock_desc, 20) != 0)
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

bool SubServer::send(LVData * data)
{
    int sent = 0;
    int size_of_data;
    int width, height;
    std::cout << "Getting rgb data" << std::endl;
    uint8_t * lv_rgb = data->get_rgb(&size_of_data, &width, &height);
    uint32_t send_dimensions = ((0xFFFF & width) << 16) | (0xFFFF & height);
    
    std::cout << "Sending size" << std::endl;
    sent = ::send(temp_sock_desc, &size_of_data, 4, 0);
    ::send(temp_sock_desc, &send_dimensions, 4, 0);
    sent = 0;
    while(sent < size_of_data) 
    {
        int bytes_sent = 0;
        bytes_sent = ::send(temp_sock_desc, lv_rgb+sent, size_of_data-sent, 0);
        std::cout << "Sent: " << bytes_sent << std::endl;
        if(bytes_sent == -1) {
            std::cout << "Cannot write to server!" << std::endl;
            free(lv_rgb);
            return false;
        }
        sent += bytes_sent;
    }
    
    free(lv_rgb);
    return true;
}

int8_t * SubServer::recv(uint32_t * size)
{
    int recvd = 0;
    ::recv(temp_sock_desc, size, 4, 0);
    int8_t * out = new int8_t[*size];
    while(recvd < *size) 
    {
        int bytes_recvd;
        bytes_recvd = ::recv(temp_sock_desc, out+recvd, *(size)-recvd, 0);
        std::cout << "Received: " << bytes_recvd << std::endl;
        if(bytes_recvd == -1) {
            std::cout << "Cannot receive data!" << std::endl;
            return out;
        }
        recvd += bytes_recvd;
    }
    return out;
}

void SubServer::disconnect()
{
    close(sock_desc);
    close(temp_sock_desc);
}
