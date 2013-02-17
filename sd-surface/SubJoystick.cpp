//The headers
#include "SDL/SDL.h"
#include <iostream>
#include <string>
#include "SubJoystick.hpp"

using namespace std;

SubJoystick::SubJoystick()
{
    //Initialize the commands (and make sure they all start at 0)
    bzero(commands, 9);
    //forward/back||left/right||pitch up/down||zoom out||zoom in||descend||ascend||take picture//lights
    
}

void SubJoystick::handle_input(SDL_Event event)
{
    //If a axis was changed
    if( event.type == SDL_JOYAXISMOTION )
    {
        //If the first joystick has moved
        if( event.jaxis.which == 0 )
        {
            //Forward/Backward (commands[0])
            if( event.jaxis.axis == 1 )
            {
                //If the F/R axis is neutral
                if( ( event.jaxis.value > -8000 ) && ( event.jaxis.value < 8000 ) )
                {
                    commands[FORWARD] = 0; 
                }
                //If not
                else
                {
                    //Change Direction
                    if( event.jaxis.value < 0 )
                    {
                        commands[FORWARD] = -1; //backward
                    }
                    else
                    {
                        commands[FORWARD] = 1; //forward
                    }
                }
            }
            //Left/Right (commands[1])
            else if( event.jaxis.axis == 3 )
            {
                //If the X axis is neutral
                if( ( event.jaxis.value > -8000 ) && ( event.jaxis.value < 8000 ) )
                {
                    commands[LEFT] = 0;
                }
                //If not
                else
                {
                    //Change Direction
                    if( event.jaxis.value < 0 )
                    {
                        commands[LEFT] = -1; //turn right
                    }
                    else
                    {
                        commands[LEFT] = 1;
                    }
                }
            }
            //Pitch (commands[2])
            else if( event.jaxis.axis == 4 )
            {
                //If the axis is neutral
                if( ( event.jaxis.value > -8000 ) && ( event.jaxis.value < 8000 ) )
                {
                    commands[PITCH] = 0; //stop pitch
                }
                //If not
                else
                {
                    //change direction
                    if( event.jaxis.value < 0 )
                    {
                        commands[PITCH] = -1; //pitch down
                    }
                    else
                    {
                        commands[PITCH] = 1; //pitch up
                    }
                }
            }
            //zoom in (commands[4)
            else if( event.jaxis.axis == 2 )
            {
                //If the zoom in axis is neutral and we're not zoooming out
                if( event.jaxis.value < 8000 && commands[ZOOM] !=-1)
                {
                    commands[ZOOM] = 0; 
                }
                else
                {
                    commands[ZOOM] = 1; //Zoom In
                }
            }
            //zoom out (commands[3])
            else if( event.jaxis.axis == 5 )
            {
                //If the zoom out axis is neutral
                if(  event.jaxis.value < 8000)
                {
                    commands[ZOOM] = 0; 
                }
                else
                {
                    commands[ZOOM] = -1; //Zoom out
                }
            }
        }
    }
    //buttons pressed
    if(event.type == SDL_JOYBUTTONDOWN) {
        
        //taking a picture has priority
        if ( event.jbutton.button == 5 ) 
        {
            commands[SHOOT] = 1; //Take Picture
        }
        //lights (toggle on button down)
        else if(event.jbutton.button == 4)
        {
            if(commands[LIGHTS] == 1) 
            {
                commands[LIGHTS] = 0; //light off
            } 
            else
            {
                commands[LIGHTS] = 1; //Light on
            }
        }
        else if (event.jbutton.button == 3) 
        {
            commands[ASCEND] = 1; //ascend
        }
        else if (event.jbutton.button == 0) {
            commands[ASCEND] = -1; //decend
        }
    }
    //buttons released
    if(event.type == SDL_JOYBUTTONUP) {
        //taking a picture has priority
        if ( event.jbutton.button == 5 ) 
        {
            commands[SHOOT] = 0; //Take Picture
        }
        //lights only change on button down 
        else if (event.jbutton.button == 3) 
        {
            commands[ASCEND] = 0; //ascend
        }
        else if (event.jbutton.button == 0) {
            commands[ASCEND] = 0; //decend
        }
    }
}

//this method gets the joystick data for use elsewhere
int8_t *SubJoystick::get_data() {
    
    int8_t data_length = sizeof(commands);
    int8_t *to_send = new int8_t[10];
    to_send[0] = data_length;
    for(int8_t i=0;i<9;i++) 
    {
        to_send[i+1] = commands[i];
    }
    
    return to_send;
}
