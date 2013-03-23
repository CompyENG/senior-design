#include <SDL/SDL.h>
#include <iostream>
#include <string>

#include "../common/SignalHandler.hpp"
#include "surface.hpp"
#include "SubJoystick.hpp"
#include "SurfaceClient.hpp"

int main(int argv, char * argc[]) {
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
    
    //Create a Client
    SurfaceClient mySurfaceClient;
    
    //Connect to server
    if(mySurfaceClient.connect("pi-submarine",50000) == false)
    {
		std::cout << "Cound not connect to server." << std::endl;
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


    //While the user hasn't quit
    while( signalHandler.gotExitSignal() == false && quit == false )
    {
        //While there's events to handle
        while( SDL_PollEvent( &event ) )
        {
            //Handle events for the sub (must pass event variable)
            mySubJoystick.handle_input(event);
            
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
            mySurfaceClient.send(SubJoystick::COMMAND_LENGTH, nav_data);
            
            delete[] nav_data;
            
            // TODO: RECEIVE DATA, PROCESS, DISPLAY

            //If the user has Xed out the window
            if( event.type == SDL_QUIT )
            {
                //Quit the program
                quit = true;
            }
        }
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
