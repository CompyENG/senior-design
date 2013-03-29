#ifndef LIBPTP_PP_PTPNETWORK_H_
#define LIBPTP_PP_PTPNETWORK_H_

#include <string>
#include <stdint.h>
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>
#include "IPTPComm.hpp"

/**
 * This class will provide PTP communication over a network socket.  This code 
 * will allow either a server or a client, and the caller is responsible for
 * choosing which to make it, and handling the PTP transactions correctly.
 */
namespace PTP {
    
    class PTPNetwork : public IPTPComm {
        private:
            struct sockaddr_in client;
            struct sockaddr_in server;
            int server_sock;
            int client_sock;
            void init();
            
        public:
            enum NetworkErrors {
                ERR_CREATE = 1,
                ERR_CONNECT,
                ERR_BIND,
                ERR_LISTEN,
                ERR_ACCEPT,
                ERR_SET_SEND_TIMEOUT,
                ERR_SET_RECV_TIMEOUT,
                ERR_SEND,
                ERR_RECV,
                ERR_IP
            };
            PTPNetwork();
            PTPNetwork(std::string server, int port);
            PTPNetwork(int port);
            ~PTPNetwork();
            bool connect(std::string server, int port);
            bool listen(int port);
            bool is_client();
            bool is_server();
            virtual bool _bulk_write(const unsigned char * bytestr, const int length, const int timeout);
            virtual bool _bulk_read(unsigned char * data_out, const int size, int * transferred, const int timeout);
            virtual bool is_open();
    };
    
}

#endif /* LIBPTP_PP_PTPNETWORK_H_ */
