#ifndef SURFACECLIENT
#define SURFACECLIENT

#include <stdint.h>
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>

class SurfaceClient
{
    private:
    struct sockaddr_in client;
    int sock_desc;
    
	public:
	bool connect(const std::string& host, int port);
	uint8_t * recv(uint32_t * size_out, int16_t * width_out, int16_t * height_out, bool * success_out);
	bool send(uint32_t size_of_data, int8_t * data);
	void disconnect();
    bool check_ready();
};

#endif /* SURFACECLIENT */
