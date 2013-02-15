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
					commands[0] = 0;
				}
				//If not
				else
				{
					//Change Direction
					if( event.jaxis.value < 0 )
					{
						commands[0] = -1;
					}
					else
					{
						commands[0] = 1;
					}
				}
			}
			//Left/Right (commands[1])
			else if( event.jaxis.axis == 3 )
			{
				//If the X axis is neutral
				if( ( event.jaxis.value > -8000 ) && ( event.jaxis.value < 8000 ) )
				{
					commands[1] = 0;
				}
				//If not
				else
				{
					//Change Direction
					if( event.jaxis.value < 0 )
					{
						commands[1] = -1;
					}
					else
					{
						commands[1] = 1;
					}
				}
			}
			//Pitch (commands[2])
			else if( event.jaxis.axis == 4 )
			{
				//If the axis is neutral
				if( ( event.jaxis.value > -8000 ) && ( event.jaxis.value < 8000 ) )
				{
					//stop pitch
					commands[2] = 0;
				}
				//If not
				else
				{
					//change direction
					if( event.jaxis.value < 0 )
					{
						commands[2] = -1;
					}
					else
					{
						commands[2] = 1;
					}
				}
			}
			//zoom in (commands[4)
			else if( event.jaxis.axis == 2 )
			{
				//If the zoom in axis is neutral
				if( event.jaxis.value < 8000 )
				{
					commands[4] = 0;
				}
				//If not we're zooming in
				else
				{
					commands[4] = 1;
				}
			}
			//zoom out (commands[3])
			else if( event.jaxis.axis == 5 )
			{
				//If the zoom out axis is neutral
				if(  event.jaxis.value < 8000  )
				{
					commands[3] = 0;
				}
				//If not we're zooming out
				else
				{
					commands[3] = 1;
				}
			}
		}
	}
	//buttons pressed
	if(event.type == SDL_JOYBUTTONDOWN) {
		
		//taking a picture has priority
		if ( event.jbutton.button == 5 ) 
		{
			commands[7] = 1;
		}
		//lights (toggle on button down)
		else if(event.jbutton.button == 4)
		{
			if(commands[8] == 1) 
			{
				commands[8] = 0;
			} 
			else
			{
				commands[8] = 1;
			}
		}
		//ascend
		else if (event.jbutton.button == 3) 
		{
			commands[6] = 1;
		}
		//decend
		else if (event.jbutton.button == 0) {
			commands[5] = 1;
		}
	}
	//buttons released
	if(event.type == SDL_JOYBUTTONUP) {
		
		//taking a picture has priority
		if ( event.jbutton.button == 5 ) 
		{
			commands[7] = 0;
		}
		//lights only change on button down
		//ascend
		else if (event.jbutton.button == 3) 
		{
			commands[6] = 0;
		}
		//decend
		else if (event.jbutton.button == 0) {
			commands[5] = 0;
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
