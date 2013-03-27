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

#define MAGIC_REQ 0xF061
#define MAGIC_RESP 0xA542

/*
SubClient::Subserver()
{
} */

bool SubServer::listen(int port)
{
    sock_desc = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock_desc == -1)
    {
        std::cout << "cannot create socket!" << std::endl;
        return false;
    }
    memset(&server, 0, sizeof(server));  
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;  
    server.sin_port = htons(50000);  
    if (::bind(sock_desc, (struct sockaddr*)&server, sizeof(server)) != 0)
    {
        std::cout << "cannot bind socket!" << std::endl;
        close(sock_desc);  
        return false;
    }
    
    if (::listen(sock_desc, 20) != 0)
    {
        std::cout << "cannot listen on socket!" << std::endl;
        close(sock_desc);  
        return false;
    }
    
    memset(&client, 0, sizeof(client));  
    socklen_t len = sizeof(client);
    temp_sock_desc = accept(sock_desc, (struct sockaddr*)&client, &len);  
    if (temp_sock_desc == -1)
    {
        std::cout << "cannot accept client!" << std::endl;
        close(sock_desc);  
        return 0;
    }
    
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if(setsockopt(sock_desc, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof tv))
    {
        std::cout << "cannot set recv timeout!" << std::endl;
        return false;
	}
    if(setsockopt(sock_desc, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,  sizeof tv))
    {
        std::cout << "cannot set send timeout!" << std::endl;
        return false;
    }   
    
    
    std::cout << "Server running on PORT: " << port << std::endl;
    return true;
}

bool SubServer::send(LVData& data)
{
    int sent = 0;
    int size_of_data;
    int width, height;
    std::cout << "Getting rgb data" << std::endl;
    uint8_t * lv_rgb = data.get_rgb(&size_of_data, &width, &height);
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
	ERROR_TIMEOUT = 0;
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
            ERROR_TIMEOUT = 1;
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

// Receives two bytes, and checks if it receives MAGIC_REQ. If not, return false
//  If we get MAGIC_REQ, send MAGIC_RESP, and ensure we sent two bytes
//  If anything goes wrong, return false
bool SubServer::reply_ready()
{
    int bytes_recvd;
    uint16_t recvd = 0;
    bytes_recvd = ::recv(temp_sock_desc, &recvd, 2, 0);
    if(bytes_recvd < 2) {
        return false;
    } else if(ntohs(recvd) == MAGIC_REQ) {
        int bytes_sent;
        uint16_t send_d = htons(MAGIC_RESP);
        bytes_sent = ::send(temp_sock_desc, &send_d, 2, 0);
        if(bytes_sent < 2) {
            return false;
        } else {
            return true;
        }
    }
    return false;
}
