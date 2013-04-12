//Test receiving data with the SubServer Class

#include "SubServer.hpp"

#include <iostream>
#include <string>

int main(int argv, char * argc[]) {

	//Create a Server
    SubServer mySubServer;
    
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
    

	//start server
	if(mySubServer.listenForClient(50000) == false) 
	{
		std::cout << "Could not start server" << std::endl;
		return 2;
	}
	
	std::cout << "Connected to Surface" << std::endl;
	
	//Receive command
	int8_t nav_data = mySubServer.receiveCommands();
	
	//print Command
	std::cout << "nav_data[FORWARD] = " << nav_data[0] << std::endl;
	std::cout << "nav_data[LEFT] = " << nav_data[1] << std::endl;
	std::cout << "nav_data[PITCH] = " <<  nav_data[2] << std::endl;
	std::cout << "nav_data[ZOOM] = " <<  nav_data[3] << std::endl;
	std::cout << "nav_data[ASCEND] = " <<  nav_data[4] << std::endl;
	std::cout << "nav_data[SHOOT] = " <<  nav_data[5] << std::endl;
	std::cout << "nav_data[LIGHTS] = " <<  nav_data[6] << std::endl;
	
	
	//disconnect from sever
	mySubServer.disconnectFromClient();
	
}
