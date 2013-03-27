//The headers
#include <iostream>
#include <stdint.h>
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <stdio.h>  
#include <string.h>  
#include <cstring>
#include <stdlib.h> 
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include "SurfaceClient.hpp"

#define MAGIC_REQ 0xF061
#define MAGIC_RESP 0xA542

/*
SurfaceClient::SurfaceClient()
{
} */

bool SurfaceClient::connect(const std::string& host, int port) 
{
    sock_desc = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock_desc == -1)
    {
        std::cout << "cannot create socket!\n" << std::endl;
        return false;
    }
    
    // Perform a lookup of the hostname
    struct hostent *he;
    he = gethostbyname(host.c_str());
    if(he == NULL || he->h_length == 0) {
        std::cout << "Couldn't find IP for host " << host << std::endl;
        return false;
    }

    memset(&client, 0, sizeof(client));  
    client.sin_family = AF_INET;
    memcpy(&client.sin_addr, he->h_addr_list[0], he->h_length);
    client.sin_port = htons(port);
    
    std::cout << "Host: " << host << " PORT: " << port << std::endl;
    if(::connect(sock_desc, (struct sockaddr*)&client, sizeof(client)) != 0 )
    {
        close(sock_desc);
        return false;
    }
    return true;
}

bool SurfaceClient::send(uint32_t size_of_data, int8_t * data)
{

    int sent = 0;
    sent = ::send(sock_desc, &size_of_data, 4, 0);
    sent = 0;
    while(sent < size_of_data) 
    {
        int bytes_sent = 0;
        bytes_sent = ::send(sock_desc, data+sent, size_of_data-sent, 0);
        std::cout << "Sent: " << bytes_sent << std::endl;
        if(bytes_sent == -1) {
            std::cout << "Cannot write to server!" << std::endl;
            return false;
        }
        sent += bytes_sent;
    }
    return true;
}

uint8_t * SurfaceClient::recv(uint32_t * size_out, int16_t * width_out, int16_t * height_out, bool * success_out) 
{
    uint8_t * out;
    int recvd = 0;
    uint32_t size;
    ::recv(sock_desc, &size, 4, 0);
    uint32_t dimensions;
    ::recv(sock_desc, &dimensions, 4, 0);
    *width_out = (dimensions & 0xFFFF0000) >> 16;
    *height_out = (dimensions & 0xFFFF);
    
    out = (uint8_t *)malloc(size);
    
    while(recvd < size) 
    {
        int bytes_recvd;
        bytes_recvd = ::recv(sock_desc, out+recvd, size-recvd, 0);
        std::cout << "Received: " << bytes_recvd << std::endl;
        if(bytes_recvd == -1) {
            std::cout << "Cannot receive data!" << std::endl;
            free(out);
            *success_out = false;
            return NULL;
        }
        recvd += bytes_recvd;
    }
    
    *success_out = true;
    return out;
}

void SurfaceClient::disconnect()
{
    close(sock_desc);
}

// Send MAGIC_REQ, and return false if we fail to do so
// If successful, receive two bytes, and return true if we get MAGIC_RESP
// Return false on any failure
bool SurfaceClient::check_ready() {
    uint16_t send_d = MAGIC_REQ;
    int bytes_sent = 0;
    bytes_sent = ::send(sock_desc, &send_d, 2, 0);
    if(bytes_sent < 2) {
        return false;
    } else {
        uint16_t recvd = 0;
        int bytes_recvd = 0;
        bytes_recvd = ::recv(sock_desc, &recvd, 2, 0);
        if(bytes_recvd < 2) {
            return false;
        } else if(recvd == MAGIC_RESP) {
            return true;
        }
    }
    return false;
}
