#ifndef SUBSERVER
#define SUBSERVER

#include <stdint.h>
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>
#include <libptp++.hpp>

class SubServer
{
    private:
    struct sockaddr_in server;
    struct sockaddr_in client;
    int temp_sock_desc;
    int sock_desc;
    
    public:
    int ERROR_TIMEOUT;
    bool listen(int port);
    int8_t * recv(uint32_t * size);
    bool send(LVData& data);
    void disconnect();
    bool reply_ready();
};

#endif /* SUBSERVER */
