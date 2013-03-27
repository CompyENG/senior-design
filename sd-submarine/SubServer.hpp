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
    enum Exceptions {
		ERR_CREATE = 1,
		ERR_CONNECT,
		ERR_BIND,
		ERR_LISTEN,
		ERR_ACCEPT,
		ERR_SET_SEND_TIMEOUT,
		ERR_SET_RECV_TIMEOUT,
		ERR_SEND,
		ERR_RECV };
    bool listen(int port);
    int8_t * recv(uint32_t * size);
    bool send(LVData& data);
    void disconnect();
    bool reply_ready();
};

#endif /* SUBSERVER */
