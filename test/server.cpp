#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
 
using namespace std;
 
int main()
{
        socklen_t sockfd, clientsock, sin_size;
        //int len1, len2, len3;
 
        char *buf;
 
        int bind_check, list_check; //variables used in error checking
 
        //char *msg1 = "Connected to server...";
        //char *msg2 = "Initializing data transfer...";
        //char *msg3 = "Ready. Commence data transfer.";
        //len1 = strlen(msg1);
        //len2 = strlen(msg2);
        //len3 = strlen(msg3);
 
        struct sockaddr_in server_addr;
        struct sockaddr_in client_addr;
        sockfd=socket(PF_INET, SOCK_STREAM, 0 );
        if ( sockfd == -1 )
        {
                cout<<"Could not bind socket.\n";
                return 1;
        }
 
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(31337);
        server_addr.sin_addr.s_addr = INADDR_ANY;
        memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);
 
        bind_check=bind( sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr) );
        if ( bind_check == -1 )
        {
                cout<<"Could not bind socket.\n";
        }
        list_check=listen( sockfd, 5 );
        if( list_check == -1 )
        {
                cout<<"Could not listen() on the socket"<<sockfd<<"\n";
        }
 
        sin_size = sizeof( struct sockaddr_in );
 
        clientsock=accept( sockfd, (struct sockaddr *)&client_addr, &sin_size );
 
        while ( buf != "exit" )
        {
                if ( recv( clientsock, buf, 500, 0 ) == 0 )
                {
                        cout<<"Remote host closed connection. Firewall?";
                }
        }
        close(sockfd);
        close(clientsock);
}
