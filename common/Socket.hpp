// Definition of the Socket class

#ifndef Socket_class
#define Socket_class


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>

typedef struct {
    uint32_t length;
    uint16_t width;
    uint16_t height;
    uint8_t * data;
} live_view_data;

class Socket
{
    public:
        const int MAXHOSTNAME = 200;
        const int MAXCONNECTIONS = 5;
        const int MAXRECV = 500;
        Socket();
        virtual ~Socket();

        // Server initialization
        bool create();
        bool bind ( const int port );
        bool listen() const;
        bool accept ( Socket& ) const;

        // Client initialization
        bool connect ( const std::string host, const int port );

        // Data Transimission
        bool send ( const std::string ) const;
        int recv ( std::string& ) const;
        bool send ( const live_view_data ) const;
        int recv ( live_view_data * ) const;


        void set_non_blocking ( const bool );

        bool is_valid() const { return m_sock != -1; }

    private:
        int m_sock;
        sockaddr_in m_addr;
};


#endif
