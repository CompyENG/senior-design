#include <string>
#include <stdint.h>
#include <iostream>
#include <stdint.h>
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include "PTPNetwork.hpp"
#include <unistd.h>

namespace PTP {

PTPNetwork::PTPNetwork() {
    this->init();
}

PTPNetwork::PTPNetwork(std::string server, int port) {
    this->init();
    this->connect(server, port);
}

PTPNetwork::PTPNetwork(int port) {
    this->init();
    this->listen(port);
}

PTPNetwork::~PTPNetwork() {
    if(this->is_server()) {
        ::close(this->client_sock);
        ::close(this->server_sock);
    } else {
        ::close(this->client_sock);
    }
}

void PTPNetwork::init() {
    this->client_sock = -1;
    this->server_sock = -1;
}

bool PTPNetwork::connect(std::string server, int port) {
    this->client_sock = ::socket(AF_INET, SOCK_STREAM, 0); 
    if (this->client_sock == -1)
    {
        throw PTPNetwork::ERR_CONNECT;
        return false;
    }
    
    // Perform a lookup of the hostname
    struct hostent *he;
    he = ::gethostbyname(server.c_str());
    if(he == NULL || he->h_length == 0) {
        throw PTPNetwork::ERR_IP;
        return false;
    }

    std::memset(&this->server, 0, sizeof(this->server));  
    this->server.sin_family = AF_INET;
    std::memcpy(&this->server.sin_addr, he->h_addr_list[0], he->h_length);
    this->server.sin_port = htons(port);
    
    // TODO: Move timeout set to a different location
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if(setsockopt(this->client_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof tv))
    {
        throw PTPNetwork::ERR_SET_RECV_TIMEOUT;
        return false;
	}
    if(setsockopt(this->client_sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,  sizeof tv))
    {
        throw PTPNetwork::ERR_SET_SEND_TIMEOUT;
        return false;
    }  
    
    std::cout << "Host: " << server << " PORT: " << port << std::endl;
    if(::connect(this->client_sock, (struct sockaddr*)&this->server, sizeof(this->server)) != 0 )
    {
        ::close(this->client_sock);
		throw PTPNetwork::ERR_CONNECT;
        return false;
    }
    return true;
}

bool PTPNetwork::listen(int port) {
    this->server_sock = socket(AF_INET, SOCK_STREAM, 0); 
    if (this->server_sock == -1)
    {
        throw PTPNetwork::ERR_CREATE;
        return false;
    }
    memset(&this->server, 0, sizeof(this->server));  
    this->server.sin_family = AF_INET;
    this->server.sin_addr.s_addr = INADDR_ANY;  
    this->server.sin_port = htons(port);  
    if (::bind(this->server_sock, (struct sockaddr*)&this->server, sizeof(this->server)) != 0)
    {
        ::close(this->server_sock);
        throw PTPNetwork::ERR_CONNECT;
        return false;
    }
    
    if (::listen(this->server_sock, 20) != 0)
    {
        ::close(this->server_sock);
        throw PTPNetwork::ERR_LISTEN;
        return false;
    }
    
    memset(&this->client, 0, sizeof(this->client));  
    socklen_t len = sizeof(this->client);
    this->client_sock = accept(this->server_sock, (struct sockaddr*)&this->client, &len);  
    if (this->client_sock == -1)
    {
        ::close(this->client_sock);
        ::close(this->server_sock);
        throw ERR_ACCEPT;
        return 0;
    }
    
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if(setsockopt(this->client_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof tv))
    {
        throw PTPNetwork::ERR_SET_RECV_TIMEOUT;
        return false;
	}
    if(setsockopt(this->client_sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,  sizeof tv))
    {
        throw PTPNetwork::ERR_SET_SEND_TIMEOUT;
        return false;
    }
    
    std::cout << "Server running on PORT: " << port << std::endl;
    return true;
}

bool PTPNetwork::is_server() {
    return (this->client_sock != -1 && this->server_sock != -1);
}

bool PTPNetwork::is_client() {
    return (this->server_sock == -1 && this->client_sock == -1);
}

bool PTPNetwork::_bulk_write(const unsigned char * bytestr, const int length, const int timeout) {
    // TODO: Obey timeout
    
    int sent = 0;
    while(sent < length) 
    {
        int bytes_sent = 0;
        bytes_sent = ::send(this->client_sock, bytestr + sent, length - sent, 0);
        if(bytes_sent == -1) {
            throw PTPNetwork::ERR_SEND;
            return false;
        }
        sent += bytes_sent;
    }
    return true;
}

bool PTPNetwork::_bulk_read(unsigned char * data_out, const int size, int * transferred, const int timeout) {
    // TODO: Obey timeout
    
    int recvd = 0;
    recvd = ::recv(this->client_sock, data_out, size, 0);
    if(recvd == -1) {
        throw PTPNetwork::ERR_RECV;
        return false;
    }
    *transferred = recvd;
}

bool PTPNetwork::is_open() {
    return (this->is_server() || this->is_client());
}

}
