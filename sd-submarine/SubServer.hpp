#ifndef SUBSERVER
#define SUBSERVER

#include <stdint.h>
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>

class SubServer
{
    private:
    struct sockaddr_in server;
    struct sockaddr_in client;
    int temp_sock_desc;
    int sock_desc;
    
    public:
    bool listen(int port);
    int8_t * recv(int * size);
    bool send(LVData data);
    void disconnect();
};

#endif /* SUBSERVER */
