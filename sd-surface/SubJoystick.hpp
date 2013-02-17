#ifndef SUBJOYSTICK
#define SUBJOYSTICK

#include "SDL/SDL.h"
#include <stdint.h>

using namespace std;

class SubJoystick
{
    private:
    int8_t commands[9]; //The commands to send
	
	enum SubCommand {
		FORWARD = 0, // 1 for forward, -1 for backward, 0 for neither 
		LEFT, // 1 for left, -1 for right, 0 for neither
		PITCH, // 1 for up, -1 for down, 0 for neither
		ZOOM, // 1 for zoom in, -1 for zoom out, 0 for neither
		ASCEND, // 1 for ascend, -1 for descend, 0 for neither
		SHOOT, // 1 for "take a picture", 0 for don't
		LIGHTS // 1 when lights should be on, 0 when lights should be off
	};

    public:
    SubJoystick(); //Initializes
    int8_t * get_data(); //gets data to send
    void handle_input(SDL_Event myevent); //Handles joystick

};

#endif /* SUBJOYSTICK */
