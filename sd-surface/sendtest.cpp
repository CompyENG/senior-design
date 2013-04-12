//Test sending data with the SubClient Class

#include "SubClient.hpp"

#include <iostream>
#include <string>

int main(int argv, char * argc[]) {

	//Create a Client
    SubClient mySubClient;
    
    //make commands easier to read
    enum SubCommand {
		FORWARD = 0, // 1 for forward, -1 for backward, 0 for neither 
		LEFT, // 1 for left, -1 for right, 0 for neither
		PITCH, // 1 for up, -1 for down, 0 for neither
		ZOOM, // 1 for zoom in, -1 for zoom out, 0 for neither
		ASCEND, // 1 for ascend, -1 for descend, 0 for neither
		SHOOT, // 1 for "take a picture", 0 for don't
		LIGHTS // 1 when lights should be on, 0 when lights should be off
	};
	
	//generate fake command
    int8_t commands[7];
    commands[FORWARD] = 1;
    commands[LEFT] = 0;
    commands[PITCH] = 0;
    commands[ZOOM] = 0;
    commands[ASCEND] = 0;
    commands[SHOOT] = 0;
    commands[LIGHTS] = 0;
    

	//connect to 
	if(mySubClient.connectToSub("127.0.0.1", 50000) == false) 
	{
		std::cout << "Sub Connection Failed" << std::endl;
		return 2;
	}
	
	std::cout << "Connected to Sub" << std::endl;
	
	//Send Fake Command
	mySubClient.sendCommands(commands);
	
	//disconnect from sever
	mySubClient.disconnectFromSub();
	
}
