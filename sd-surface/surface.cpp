#include <SDL/SDL.h>
#include <iostream>
#include <string>

#include "../common/SignalHandler.hpp"
#include "surface.hpp"
#include "SubJoystick.hpp"
#include "SurfaceClient.hpp"

int main(int argc, char * argv[]) {
    SDL_Surface * screen = NULL;
    SDL_Surface * surf_lv = NULL;
    bool quit = false; // Optional SDL_QUIT handler -- We can also use this as a shutdown from the joystick
    // Set up signal handler
    SignalHandler signalHandler;
    try {
        signalHandler.setupSignalHandlers();
    } catch(int e) {
        std::cout << "Fatal error: Unable to setup signal handler. Exception: " << e << std::endl;
        return 1;
    }

    //The event structure
    SDL_Event event;
    
    //Initialize
    if( init() == false )
    {
        std::cout << "init() failed" << std::endl;
        return 2;
    }
    
    screen = SDL_SetVideoMode( 640, 480, 16, SDL_FULLSCREEN | SDL_SWSURFACE );
    
    //Create a Client
    SurfaceClient mySurfaceClient;
    
    //Connect to server
    try {
		mySurfaceClient.connect("pi-submarine",50000) == false)
	} catch(int e) {
		std::cout << "Fatal Error: Could not connect to socket. Exception: " << e << std::endl;
		return 2;
	}
	std::cout << "Connection Successful" << std::endl;
    
    //Check if there's any joysticks
    if( SDL_NumJoysticks() < 1 )
    {
        std::cout << "No Joysticks Found" << std::endl;
        return 3;
    }

    //Open the joystick
    SDL_Joystick *stick = SDL_JoystickOpen( 0 );

    //If there's a problem opening the joystick
    if( stick == NULL )
    {
        std::cout << "Could not open Joystick" << std::endl;
        return 4;
    }

    //Make the Sub
    SubJoystick mySubJoystick;
    
    //Wait for both the sub and surface to be ready
    wait_for_ready();

    //While the user hasn't quit
    while( signalHandler.gotExitSignal() == false && quit == false )
    {
        //While there's events to handle
        while( SDL_PollEvent( &event ) )
        {
            //Handle events for the sub (must pass event variable)
            mySubJoystick.handle_input(event);

            //If the user has Xed out the window
            if( event.type == SDL_QUIT )
            {
                //Quit the program
                quit = true;
            }
        }
        
        int8_t *nav_data = mySubJoystick.get_data();
            
        std::cout << "nav_data[FORWARD] = " << (int) nav_data[SubJoystick::FORWARD] << std::endl;
        std::cout << "nav_data[LEFT] = " << (int) nav_data[SubJoystick::LEFT] << std::endl;
        std::cout << "nav_data[PITCH] = " << (int) nav_data[SubJoystick::PITCH] << std::endl;
        std::cout << "nav_data[ZOOM] = " << (int) nav_data[SubJoystick::ZOOM] << std::endl;
        std::cout << "nav_data[ASCEND] = " << (int) nav_data[SubJoystick::ASCEND] << std::endl;
        std::cout << "nav_data[SHOOT] = " << (int) nav_data[SubJoystick::SHOOT] << std::endl;
        std::cout << "nav_data[LIGHTS] = " << (int) nav_data[SubJoystick::LIGHTS] << std::endl;
        
        // TODO: SEND DATA HERE
        
        //Send data to sub
        try {
			mySurfaceClient.send(SubJoystick::COMMAND_LENGTH, nav_data);
		} catch(int e) {
			std::cout << "Error: Could not send data. Exception: " << e << std::endl;
			std::cout << "Wating for ready" << std::endl;
			wait_for_ready(mySurfaceClient, signalHandler);
			continue;
		}
        
        std::cout << "Sent data" << std::endl;
        
        delete[] nav_data;
        
        // TODO: RECEIVE DATA, PROCESS, DISPLAY
        uint8_t * lv_rgb;
        uint32_t lv_size;
        int16_t width, height;
        bool success;
        try {
            lv_rgb = mySurfaceClient.recv(&lv_size, &width, &height, &success);
	    } catch(int e) {
			std::cout << "Error: Could not receive data. Exception: " << e << std::endl;
			std::cout << "Wating for ready" << std::endl;
			wait_for_ready(mySurfaceClient, signalHandler);
			continue;
		}
        if(success) {
            std::cout << "Received data -- displaying" << std::endl;
            surf_lv = SDL_CreateRGBSurfaceFrom(lv_rgb, width, height, 24, width * 3, 0x0000ff, 0x00ff00, 0xff0000, 0);
            
            // Apply image to screen
            SDL_BlitSurface(surf_lv, NULL, screen, NULL);
            
            SDL_Flip(screen);
            
            SDL_FreeSurface(surf_lv);
        }
        free(lv_rgb);
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

void wait_for_ready(SurfaceClient& client,SignalHandler& sigHand) {
	while(client.check_ready() == false && sigHand.gotExitSignal() == false);
}
