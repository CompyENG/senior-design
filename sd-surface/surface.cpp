#include <SDL/SDL.h>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../common/SignalHandler.hpp"
#include "surface.hpp"
#include "SubJoystick.hpp"

#define BUFFER_SIZE 2048

int main(int argv, char * argc[]) {
    SDL_Surface *screen = NULL;
    bool quit = false; // Optional SDL_QUIT handler -- We can also use this as a shutdown from the joystick
    // Set up signal handler
    SignalHandler signalHandler;
    try {
        signalHandler.setupSignalHandlers();
    } catch(int e) {
        cout << "Fatal error: Unable to setup signal handler. Exception: " << e << endl;
        return 1;
    }

    //The event structure
    SDL_Event event;
    
    //Initialize
    if( init() == false )
    {
        cout << "init() failed" << endl;
        return 2;
    }
    
    // Set up screen
    screen = SDL_SetVideoMode( 640, 480, 32, SDL_SWSURFACE );
    
    //Check if there's any joysticks
    if( SDL_NumJoysticks() < 1 )
    {
        cout << "No Joysticks Found" << endl;
        return 3;
    }

    //Open the joystick
    SDL_Joystick *stick = SDL_JoystickOpen( 0 );

    //If there's a problem opening the joystick
    if( stick == NULL )
    {
        cout << "Could not open Joystick" << endl;
        return 4;
    }

    //Make the Sub
    SubJoystick mySubJoystick;
    
    // Connect to the submarine
    int sock, bytes_recieved;
    struct hostent *host;
    struct sockaddr_in server_addr;

    host = gethostbyname("pi-submarine");

    sock = socket(AF_INET, SOCK_STREAM,0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(50000);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(server_addr.sin_zero),8);

    connect(sock, (struct sockaddr *)&server_addr,sizeof(struct sockaddr));

    //While the user hasn't quit
    while( signalHandler.gotExitSignal() == false && quit == false )
    {
        int8_t *nav_data;
        //While there's events to handle
        while( SDL_PollEvent( &event ) )
        {
            //Handle events for the sub (must pass event variable)
            mySubJoystick.handle_input(event);
            
            // TODO: SEND DATA HERE
            //for(int8_t i=0;i<8;i++) {
            //    cout << "nav_data[" << (int) i << "] = " << (int) nav_data[i] << endl;
            //}
            //delete[] nav_data;
            //If the user has Xed out the window
            if( event.type == SDL_QUIT )
            {
                //Quit the program
                quit = true;
            }
        }
        
        nav_data = mySubJoystick.get_data();
            
        uint32_t send_size = (uint32_t)nav_data[0];
        send(sock, &send_size, 4, 0);
        send(sock, nav_data, send_size, 0);
        delete[] nav_data;
        // Get live view data
        // Receive size
        uint32_t recv_size = 0;
        uint16_t width, height;
        recv(sock, &recv_size, 4, 0); // Size is four bytes long
        recv(sock, &width, 2, 0); // Width is two bytes long
        recv(sock, &height, 2, 0); // Height is two bytes long
        cout << "Going to receive: " << recv_size << endl;
        cout << "Width: " << width << " ; Height: " << height << endl;
        //int bytes_recvd;
        uint8_t * lv_data = (uint8_t *)malloc(recv_size);
        int recvd = 0;
        while(recvd < recv_size) {
            if(recv_size-recvd < BUFFER_SIZE) {
                recvd += recv(sock, lv_data+recvd, recv_size-recvd, 0);
            } else {
                recvd += recv(sock, lv_data+recvd, BUFFER_SIZE, 0);
            }
        }
        // Put in a surface
        surf_lv = SDL_CreateRGBSurfaceFrom(lv_data, width, height, 24, width*3, 0x0000ff, 0x00ff00, 0xff0000, 0);
        
        //Apply image to screen
        SDL_BlitSurface( surf_lv, NULL, screen, NULL );
        
        SDL_Flip( screen );
        
        //SDL_Delay(10);
        
        SDL_FreeSurface( surf_lv );
        free(lv_data);

        
    }

    //Clean up
    clean_up(stick);

    return 0;
}

bool init()
{
    //Initialize all SDL subsystems
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        return false;
    }

    //If everything initialized fine
    return true;
}

void clean_up(SDL_Joystick *stick)
{
    //Close the joystick
    SDL_JoystickClose( stick );

    //Quit SDL
    SDL_Quit();
}
