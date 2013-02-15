#ifndef SUBJOYSTICK
#define SUBJOYSTICK

#include "SDL/SDL.h"
#include <stdint.h>

using namespace std;

class SubJoystick
{
    private:
    int8_t commands[9]; //The commands to send

    public:
    SubJoystick(); //Initializes
    int8_t * get_data(); //gets data to send
    void handle_input(SDL_Event myevent); //Handles joystick

};

#endif /* SUBJOYSTICK */
