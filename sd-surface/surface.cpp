#include <SDL/SDL.h>
#include <iostream>
#include <string>
#include <libptp++/libptp++.hpp>

#include "../common/SignalHandler.hpp"
#include "surface.hpp"
#include "SubJoystick.hpp"
#include "../common/SDDefines.hpp"

int main(int argc, char * argv[]) {
    // These variables can be used as dummy placeholders when we don't need a parameter to ptp_transaction
    PTP::PTPContainer in_data, out_resp, out_data;
    SDL_Surface * screen = NULL;
    SDL_Surface * surf_lv = NULL;
    SDL_Joystick *stick = NULL;
    bool quit = false; // Optional SDL_QUIT handler -- We can also use this as a shutdown from the joystick
    bool select = false;
    bool debug = false;
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
        std::cout << "Fatal error: init() failed" << std::endl;
        return 2;
    }
    
    if(argc > 1) {
        screen = SDL_SetVideoMode( 640, 480, 16, SDL_SWSURFACE );
        debug = true;
    } else {
        screen = SDL_SetVideoMode( 640, 480, 16, SDL_FULLSCREEN | SDL_SWSURFACE );
    }
    
    show_image_status("/usr/share/sd-surface/controller.bmp", screen);
    
    // TODO: Error out after some amount of time?
    while(stick == NULL && signalHandler.gotAnySignal() == false) {
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
        SDL_InitSubSystem(SDL_INIT_JOYSTICK);
        //Check if there's any joysticks
        if( SDL_NumJoysticks() < 1 )
        {
            std::cout << "No Joysticks Found" << std::endl;
            continue;
        }

        //Open the joystick
        stick = SDL_JoystickOpen( 0 );

        //If there's a problem opening the joystick
        if( stick == NULL )
        {
            std::cout << "Could not open Joystick" << std::endl;
            continue; // Try again
        }
    }
    
    //Make the Sub
    SubJoystick mySubJoystick;
    
    show_image_status("/usr/share/sd-surface/connecting.bmp", screen);
    
    //Create a Client
    PTP::PTPNetwork surfaceClientBackend;
    
    //Connect to server -- TODO: Move to an FSM implementaiton in the main SDL loop and move this code below?
    //  This would allow us to exit this portion of the code cleanly using the joystick
    bool connected = false;
    while(connected == false && signalHandler.gotAnySignal() == false) {
        try {
            std::string host = "pi-submarine";
            if(argc > 1) {
                host = argv[1];
            }
            surfaceClientBackend.connect(host,50000);
            connected = true;
        } catch(PTP::PTPNetwork::NetworkErrors e) {
            std::cout << "Error: Could not connect to socket. Trying again. Exception: " << e << std::endl;
        } catch(...) {
            std::cout << "Caught some other exception.  Trying again." << std::endl;
        }
    }
    
    if(signalHandler.gotAnySignal() == true) {
        clean_up(NULL);
        if(signalHandler.gotExitSignal() == true && debug == false) {
            system ("sudo shutdown -h now");
        }
        return 1;
    }
        
    
	std::cout << "Connection Successful" << std::endl;
    PTP::CameraBase surfaceClient(&surfaceClientBackend);
    
    // Make sure we're connected to the camera
    show_image_status("/usr/share/sd-surface/camera.bmp", screen);
    bool camera_connected = false;
    while(camera_connected == false) {
        PTP::PTPContainer connect_cmd(PTP::PTPContainer::CONTAINER_TYPE_COMMAND, SD_MAGIC);
        connect_cmd.add_param(SD_REQ_CONNECTED);
        try {
            surfaceClient.ptp_transaction(connect_cmd, in_data, false, out_resp, out_data);
        } catch(PTP::LIBPTP_PP_ERRORS e) {
            std::cout << "Error in ptp_transaction: " << e << std::endl;
            continue;
        } catch(PTP::PTPNetwork::NetworkErrors e) {
            // TODO -- this is probably a fatal error
            std::cout << "Network error in ptp_transaction: " << e << std::endl;
            continue;
        }
        
        if(out_resp.code == SD_MAGIC && out_resp.get_param_n(0) == SD_IS_CONNECTED) {
            camera_connected = true;
        }
    }
    

    //While the user hasn't quit
    while( signalHandler.gotAnySignal() == false && quit == false )
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
            
        /*std::cout << "nav_data[FORWARD] = " << (int) nav_data[SubJoystick::FORWARD] << std::endl;
        std::cout << "nav_data[LEFT] = " << (int) nav_data[SubJoystick::LEFT] << std::endl;
        std::cout << "nav_data[PITCH] = " << (int) nav_data[SubJoystick::PITCH] << std::endl;
        std::cout << "nav_data[ZOOM] = " << (int) nav_data[SubJoystick::ZOOM] << std::endl;
        std::cout << "nav_data[ASCEND] = " << (int) nav_data[SubJoystick::ASCEND] << std::endl;
        std::cout << "nav_data[SHOOT] = " << (int) nav_data[SubJoystick::SHOOT] << std::endl;
        std::cout << "nav_data[LIGHTS] = " << (int) nav_data[SubJoystick::LIGHTS] << std::endl;
        std::cout << "nav_data[QUIT] = " << (int) nav_data[SubJoystick::QUIT] << std::endl;*/
        
        if(nav_data[SubJoystick::OPTION] == 1) {
            // If you hold select, different things may happen
            select = true;
        }
        
        if(nav_data[SubJoystick::QUIT] == 1) {
            quit = true;
            break;
        }
        
        // Send data
        PTP::PTPContainer joy_cmd(PTP::PTPContainer::CONTAINER_TYPE_COMMAND, SD_MAGIC);
        joy_cmd.add_param(SD_JOYDATA);
        PTP::PTPContainer joy_data(PTP::PTPContainer::CONTAINER_TYPE_DATA, SD_MAGIC);
        joy_data.set_payload(nav_data, SubJoystick::COMMAND_LENGTH);
        try {
            surfaceClient.ptp_transaction(joy_cmd, joy_data, false, out_resp, out_data);
        } catch(PTP::LIBPTP_PP_ERRORS e) {
            std::cout << "Error in ptp_transaction: " << e << std::endl;
            break;
        } catch(PTP::PTPNetwork::NetworkErrors e) {
            std::cout << "Network error in ptp_transaction: " << e << std::endl;
            break;
        }
        
        // Check response
        if(out_resp.code != SD_MAGIC || out_resp.get_param_n(0) != SD_OK) {
            std::cout << "Error: Did not send joystick data." << std::endl;
            // Something seems to be wrong -- let's continue so that we just try again
            continue;
        }
        
        int mode = out_resp.get_param_n(1);
        
        //std::cout << "Sent joystick data" << std::endl;
        
        delete[] nav_data;
        
        // Receive live view data, process, display
        uint8_t * lv_rgb;
        int lv_size;
        uint32_t width, height;
        PTP::PTPContainer lv_cmd(PTP::PTPContainer::CONTAINER_TYPE_COMMAND, SD_MAGIC);
        lv_cmd.add_param(SD_LVDATA);
        PTP::PTPContainer lv_resp, lv_data;
        surfaceClient.ptp_transaction(lv_cmd, in_data, true, lv_resp, lv_data);
        
        // Put our live view data, width, height and size in the right place
        if(lv_resp.code != SD_MAGIC || lv_resp.get_param_n(0) != SD_OK || lv_data.code != SD_MAGIC) {
            std::cout << "Error: something went wrong receiving live view data." << std::endl;
            std::cout << "       lv_resp.code: " << lv_resp.code << std::endl;
            std::cout << "       lv_resp[0]:   " << lv_resp.get_param_n(0) << std::endl;
            std::cout << "       lv_data.code: " << lv_data.code << std::endl;
            // Again, something went wrong... let's just start over and try again
            continue;
        }
        
        lv_rgb = lv_data.get_payload(&lv_size);
        width = lv_resp.get_param_n(1);
        height = lv_resp.get_param_n(2);
        
        //std::cout << "Received data -- displaying" << std::endl;
        surf_lv = SDL_CreateRGBSurfaceFrom(lv_rgb, width, height, 16, width * 2, 0xF800, 0x03E0, 0x001F, 0);
        
        SDL_SoftStretch(surf_lv, NULL, screen, NULL);

        SDL_Flip(screen);
        
        /*
        if(mode == 1) {
            draw_bmp_location("/usr/share/sd-submarine/mode-video.bmp", screen, 50, 50);
        } else {
            draw_bmp_location("/usr/share/sd-submarine/mode-picture.bmp", screen, 50, 50);
        }*/
        
        SDL_FreeSurface(surf_lv);
            
        delete[] lv_rgb;
    }
    
    // Send stop command to submarine
    PTP::PTPContainer quit_cmd(PTP::PTPContainer::CONTAINER_TYPE_COMMAND, SD_MAGIC);
    if(signalHandler.gotUpdateSignal()) {
        quit_cmd.add_param(SD_UPDATE);
    } else {
        quit_cmd.add_param(SD_QUIT);
    }
    surfaceClient.ptp_transaction(quit_cmd, in_data, false, out_resp, out_data);
    // TODO: Check response

    //Clean up
    clean_up(stick);
    if(select == false && signalHandler.gotUpdateSignal() == false && debug == false) {
        // Only shutdown if we aren't holding select, and haven't received an "update" signal
        system ("sudo shutdown -h now");
    }
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
    if(stick != NULL) {
        //Close the joystick
        SDL_JoystickClose( stick );
    }

    //Quit SDL
    SDL_Quit();
}

void show_image_status(const char * image_path, SDL_Surface * screen) {
    SDL_Surface *image;
    SDL_Rect dest;
    
    image = SDL_LoadBMP(image_path);
    if(image == NULL) {
        std::cout << "ERROR: Couldn't load image: " << image_path << std::endl;
        return;
    }
    
    dest.x = 0;
    dest.y = 0;
    dest.w = image->w;
    dest.h = image->h;
    SDL_BlitSurface(image, NULL, screen, &dest);
    
    SDL_UpdateRects(screen, 1, &dest);
}

void draw_bmp_location(const char * image_path, SDL_Surface * screen, int x, int y) {
    SDL_Surface *image;
    SDL_Rect dest;
    
    image = SDL_LoadBMP(image_path);
    if(image == NULL) {
        std::cout << "ERROR: Couldn't load image: " << image_path << std::endl;
        return;
    }
    
    dest.x = x;
    dest.y = y;
    dest.w = image->w;
    dest.h = image->h;
    SDL_BlitSurface(image, NULL, screen, &dest);
    
    SDL_UpdateRects(screen, 1, &dest);
}
