#include <iostream>
#include <unistd.h>
#include <libptp++/libptp++.hpp>
#include <libusb-1.0/libusb.h>
#include <bcm2835.h>

// We need the sub joystick class so we can reuse our data struct
#include "../sd-surface/SubJoystick.hpp"
#include "Motor.hpp"
#include "../common/SignalHandler.hpp"
#include "submarine.hpp"
#include "../common/SDDefines.hpp"

int main(int argc, char * argv[]) {
    if(argc > 1) {
        bcm2835_set_debug(1);
    }
    
    int error;
    PTP::PTPUSB proto;
    PTP::CHDKCamera cam;
    Motor subMotors[4]; // We need to control 4 motors
    SignalHandler signalHandler;
    int8_t * joy_data = NULL;
    uint32_t joy_data_len;
    int cmd;
    int8_t sub_state[SubJoystick::COMMAND_LENGTH]; // The current state of the submarine
    PTP::PTPNetwork subServerBackend;
    PTP::CameraBase subServer;
    
    bzero(sub_state, SubJoystick::COMMAND_LENGTH);
    
    // Set up our signal handler(s)
    try {
        signalHandler.setupSignalHandlers();
    } catch(int e) {
        std::cout << "Fatal error: Unable to setup signal handler. Exception: " << e << std::endl;
        return 1;
    }
    
    try {
        subServerBackend.listen(50000);
        subServer.set_protocol(&subServerBackend);
	} catch(int e) {
		std::cout << "Fatal Error: Unable to set up socket. Ex: " << e << std::endl;
	}
    
    // Keep trying to set up the camera until something tells us to stop
    while(setup_camera(cam, proto, &error) == false && signalHandler.gotAnySignal() == false) {
        std::cout << "Error setting up camera: " << error << " -- Trying again" << std::endl;
    }
    if(signalHandler.gotAnySignal() == true) {
        std::cout << "Fatal error: Failed to set up camera. Quitting. Last error: " << error << std::endl;
        return 2;
    }
    
    std::cout << "Camera is ready" << std::endl;
    std::cout << "CHDK Version: " << cam.get_chdk_version() << std::endl;
    
    // Initialize motors
    if(argc > 1) {
        Motor::setup_gpio(true);
    }
    setup_motors(subMotors);
    std::cout << "Motors are ready" << std::endl;
    
    // TODO: Signal handler to allow us to quit loop when we receive SIGUSR1
    while(signalHandler.gotAnySignal() == false) {
        // Receive a PTP container
        PTP::PTPContainer container_in;
        try {
            subServer.recv_ptp_message(container_in);
        } catch(PTP::LIBPTP_PP_ERRORS e) {
            std::cout << "Error: PTP error. Code: " << e << std::endl;
            continue;
        }
        
        std::cout << "Received container" << std::endl;
        
        // Check command
        if(container_in.type != PTP::PTPContainer::CONTAINER_TYPE_COMMAND || container_in.code != SD_MAGIC) {
            // If what we got isn't a command... or isn't for us... we're in the wrong place!
            // Let's send an error and bail
            PTP::PTPContainer response(PTP::PTPContainer::CONTAINER_TYPE_RESPONSE, SD_MAGIC);
            response.add_param(SD_ERROR);
            
            subServer.send_ptp_message(response);
            std::cout << "No command." << std::endl;
            continue;
        }
        
        // OK... we MUST have received a command.  Let's check what its param is
        uint32_t param = container_in.get_param_n(0);
        switch(param) {
            case SD_REQ_CONNECTED: {
                // For now, this is a dumb response.  In the future, let's move where
                //  we attempt to connect to the camera.  That way, there's a possibility
                //  this would say it's not connected.
                // Alternatively, this command could be required to initiate the connection
                PTP::PTPContainer response(PTP::PTPContainer::CONTAINER_TYPE_RESPONSE, SD_MAGIC);
                response.add_param(SD_IS_CONNECTED);
                
                subServer.send_ptp_message(response);
                std::cout << "Sent SD_IS_CONNECTED" << std::endl;
                break;
            }
            case SD_JOYDATA: {
                std::cout << "Got SD_JOYDATA" << std::endl;
                // Ah-ha! We've received joystick data! Let's extract it and parse it
                // First, receive the data container
                PTP::PTPContainer data;
                
                try {
                    subServer.recv_ptp_message(data);
                } catch(PTP::LIBPTP_PP_ERRORS e) {
                    std::cout << "Error getting data: " << e << std::endl;
                    break;
                }
                std::cout << "Got data -- " << data.type << std::endl;
                
                // TODO: Check transaction ID, also
                if(data.type != PTP::PTPContainer::CONTAINER_TYPE_DATA) {
                    std::cout << "Got SD_JOYDATA, but no data" << std::endl;
                    break;
                }
                
                if(joy_data != NULL) {
                    std::cout << "Deleting joy_data" << std::endl;
                    delete[] joy_data; // Need to delete old joy_data before allocating more memory
                }
                std::cout << "About to get payload" << std::endl;
                joy_data = (int8_t *)data.get_payload((int *)&joy_data_len);
                std::cout << "Got payload, updating motors" << std::endl;
                update_motors(sub_state, joy_data, joy_data_len, subMotors, cam);
                std::cout << "Updated motors" << std::endl;
                
                // TODO: Error checking
                // Everything should have gone OK, so send SD_OK response
                PTP::PTPContainer response(PTP::PTPContainer::CONTAINER_TYPE_RESPONSE, SD_MAGIC);
                response.add_param(SD_OK);
                
                subServer.send_ptp_message(response);
                std::cout << "Sent OK" << std::cout;
                break;
            }
            case SD_LVDATA: {
                // We want live view data! Let's pack it up and send it off!
                
                // First, get the live view data
                PTP::LVData lv;
                cam.get_live_view_data(lv, true);
                std::cout << "Got live view from camera" << std::endl;
                
                uint8_t * lv_rgb;
                int size, width, height;
                uint32_t size_out, width_out, height_out;
                lv_rgb = lv.get_rgb(&size, &width, &height, true);
                std::cout << "Got lv_rgb -- " << size << std::endl;
                size_out = size;
                width_out = width;
                height_out = height;
                std::cout << "size_out -- " << size_out << std::endl;
                
                // For whatever reason... send data first.
                PTP::PTPContainer out_data(PTP::PTPContainer::CONTAINER_TYPE_DATA, SD_MAGIC);
                out_data.set_payload(lv_rgb, size_out);
                subServer.send_ptp_message(out_data);
                std::cout << "Sent lv_rgb" << std::endl;
                
                // Now, send our response
                PTP::PTPContainer response(PTP::PTPContainer::CONTAINER_TYPE_RESPONSE, SD_MAGIC);
                // Param 0 is "OK", param 1 is width, param 2 is height
                response.add_param(SD_OK);
                response.add_param(width_out);
                response.add_param(height_out);
                subServer.send_ptp_message(response);
                std::cout << "Sent SD_OK" << std::endl;
                
                delete[] lv_rgb;
                
                break;
            }
            case SD_UPDATE:
            case SD_QUIT: {
                // OK! Let's get out of here! But first, let's let the surface know that we're OK with this.
                PTP::PTPContainer response(PTP::PTPContainer::CONTAINER_TYPE_RESPONSE, SD_MAGIC);
                response.add_param(SD_OK);
                subServer.send_ptp_message(response);
                
                if(param == SD_UPDATE) {
                    signalHandler.setUpdateSignal(true);
                } else {
                    signalHandler.setExitSignal(true);
                }
                std::cout << "Got SD_QUIT, sent SD_OK" << std::endl;
                break;
            }
            default: {
                // We got something else... let's just send an error
                PTP::PTPContainer response(PTP::PTPContainer::CONTAINER_TYPE_RESPONSE, SD_MAGIC);
                response.add_param(SD_ERROR);
                subServer.send_ptp_message(response);
                std::cout << "Got unkonw. Sent SD_ERROR" << std::endl;
                break;
            }
        }
    }
    
    // Deconstructor will automatically take care of closing network connection
    // TODO: Make PTPNetwork a pointer instead, so we can control when destruction happens?
    if(signalHandler.gotUpdateSignal() == false) {
        // Don't shutdown if we got the update signal
        system ("sudo shutdown -h now");
    }
    return 0;
}

bool setup_camera(PTP::CHDKCamera& cam, PTP::PTPUSB& proto, int * error) {
    // TODO: try/catch
    proto.connect_to_first();
    cam.set_protocol(&proto);
    
    cam.execute_lua("switch_mode_usb(1)", NULL); // TODO: block instead of sleep?
    sleep(1);
    cam.execute_lua("set_prop(121, 1)", NULL); // Set flash to manual adjustment
    usleep(500 * 10^3);   // Sleep for half a second -- TODO: Block instead?
    cam.execute_lua("set_prop(143, 2)", NULL); // Set flash mode to off
    usleep(500 * 10^3);
    
    return true;
}

void setup_motors(Motor * subMotors) {
    int i;
    
    for(i=0;i<4;i++) {
        subMotors[i].setup(MOTOR_PINS[i]);
    }
}

bool compare_states(int8_t * sub_state, int8_t * joy_data) {
    // Returns true if the states are the same, false otherwise
    for(int i=0; i < SubJoystick::COMMAND_LENGTH; i++) {
        if(*(sub_state+i) != *(joy_data+i)) {
            return false;
        }
    }
    
    return true;
}

void update_motors(int8_t * sub_state, int8_t * joy_data, uint32_t joy_data_len, Motor * subMotors, PTP::CHDKCamera& cam) {
    
    std::cout << "joy_data[FORWARD] = " << (int) joy_data[SubJoystick::FORWARD] << std::endl;
    std::cout << "joy_data[LEFT] = " << (int) joy_data[SubJoystick::LEFT] << std::endl;
    std::cout << "joy_data[PITCH] = " << (int) joy_data[SubJoystick::PITCH] << std::endl;
    std::cout << "joy_data[ZOOM] = " << (int) joy_data[SubJoystick::ZOOM] << std::endl;
    std::cout << "joy_data[ASCEND] = " << (int) joy_data[SubJoystick::ASCEND] << std::endl;
    std::cout << "joy_data[SHOOT] = " << (int) joy_data[SubJoystick::SHOOT] << std::endl;
    std::cout << "joy_data[LIGHTS] = " << (int) joy_data[SubJoystick::LIGHTS] << std::endl;
    
    std::cout << "sub_state[FORWARD] = " << (int) sub_state[SubJoystick::FORWARD] << std::endl;
    std::cout << "sub_state[LEFT] = " << (int) sub_state[SubJoystick::LEFT] << std::endl;
    std::cout << "sub_state[PITCH] = " << (int) sub_state[SubJoystick::PITCH] << std::endl;
    std::cout << "sub_state[ZOOM] = " << (int) sub_state[SubJoystick::ZOOM] << std::endl;
    std::cout << "sub_state[ASCEND] = " << (int) sub_state[SubJoystick::ASCEND] << std::endl;
    std::cout << "sub_state[SHOOT] = " << (int) sub_state[SubJoystick::SHOOT] << std::endl;
    std::cout << "sub_state[LIGHTS] = " << (int) sub_state[SubJoystick::LIGHTS] << std::endl;
    
    //if(compare_states(sub_state, joy_data) == false) {
        //std::cout << "State has changed." << std::endl;
        // Only run through these comparisons if our states have changed
        // Forward/backward
        if(sub_state[SubJoystick::LEFT] == 0) { // Left/right takes priority
            if(joy_data[SubJoystick::FORWARD] == 1) {
                // If we want to go forward, and we're not trying to turn
                subMotors[MOTOR_LEFT].spinForward();
                subMotors[MOTOR_RIGHT].spinForward();
                
                sub_state[SubJoystick::FORWARD] = 1;
            } else if(joy_data[SubJoystick::FORWARD] == -1) {
                // If we want to go backward, and we're not trying to turn
                subMotors[MOTOR_LEFT].spinBackward();
                subMotors[MOTOR_RIGHT].spinBackward();
                
                sub_state[SubJoystick::FORWARD] = -1;
            } else if(joy_data[SubJoystick::FORWARD] == 0) {
                // If we want to go neither forward nor backward, and we're not trying to turn
                subMotors[MOTOR_LEFT].stop();
                subMotors[MOTOR_RIGHT].stop();
                
                sub_state[SubJoystick::FORWARD] = 0;
            }
        }
        
        // Left/right
        if(joy_data[SubJoystick::LEFT] == 1) {
            // If we want to turn left
            subMotors[MOTOR_LEFT].spinBackward();
            subMotors[MOTOR_RIGHT].spinForward();
            
            sub_state[SubJoystick::LEFT] = 1;
        } else if(joy_data[SubJoystick::LEFT] == -1) {
            // If we want to turn right
            subMotors[MOTOR_LEFT].spinForward();
            subMotors[MOTOR_RIGHT].spinBackward();
            
            sub_state[SubJoystick::LEFT] = -1;
        } else if(joy_data[SubJoystick::LEFT] == 0) {
            sub_state[SubJoystick::LEFT] = 0;
            
            if(sub_state[SubJoystick::FORWARD] == 0) {
                // If we don't want to turn, and we aren't trying to move forward/backward
                subMotors[MOTOR_LEFT].stop();
                subMotors[MOTOR_RIGHT].stop();
            }
        }
        
        // Pitch up/down
        if(sub_state[SubJoystick::ASCEND] == 0) {
            // If we're not ascending or descending, we can pitch up/down
            if(joy_data[SubJoystick::PITCH] == -1) {
                // If we want to pitch down and we're not ascending or descending
                subMotors[MOTOR_TOP_FRONT].spinForward();
                subMotors[MOTOR_TOP_REAR].spinBackward();
                
                sub_state[SubJoystick::PITCH] = -1;
            } else if(joy_data[SubJoystick::PITCH] == 1) {
                // If we want to pitch up and we're not ascending or descending
                subMotors[MOTOR_TOP_FRONT].spinBackward();
                subMotors[MOTOR_TOP_REAR].spinForward();
                
                sub_state[SubJoystick::PITCH] = 1;
            } else if(joy_data[SubJoystick::PITCH] == 0) {
                // If we want to stop pitching, and we're not ascending/descending
                subMotors[MOTOR_TOP_FRONT].stop();
                subMotors[MOTOR_TOP_REAR].stop();
                
                sub_state[SubJoystick::PITCH] = 0;
            }
        }
        
        // Ascend/descend
        if(joy_data[SubJoystick::ASCEND] == -1) {
            // If we want to descend
            subMotors[MOTOR_TOP_FRONT].spinForward();
            subMotors[MOTOR_TOP_REAR].spinForward();
            
            sub_state[SubJoystick::ASCEND] = -1;
        } else if(joy_data[SubJoystick::ASCEND] == 1) {
            // If we want to ascend
            subMotors[MOTOR_TOP_FRONT].spinBackward();
            subMotors[MOTOR_TOP_REAR].spinBackward();
            
            sub_state[SubJoystick::ASCEND] = 1;
        } else if(joy_data[SubJoystick::ASCEND] == 0) {
            sub_state[SubJoystick::ASCEND] = 0;
            
            if(sub_state[SubJoystick::PITCH] == 0) {
                // If we don't want to ascend/descend, and we're not pitching
                subMotors[MOTOR_TOP_FRONT].stop();
                subMotors[MOTOR_TOP_REAR].stop();
            }
        }
        
        // Zoom in/out
        if(joy_data[SubJoystick::ZOOM] == -1) {
            // If we want to zoom out
            cam.execute_lua("click('zoom_in')", NULL);
            sub_state[SubJoystick::ZOOM] = -1;
        } else if(joy_data[SubJoystick::ZOOM] == 1) {
            // If we want to zoom in
            cam.execute_lua("click('zoom_out')", NULL);
            sub_state[SubJoystick::ZOOM] = 1;
        } else if(joy_data[SubJoystick::ZOOM] == 0) {
            sub_state[SubJoystick::ZOOM] = 0;
        }
        
        // Shoot
        if(joy_data[SubJoystick::SHOOT] == 1 && sub_state[SubJoystick::SHOOT] == 0) {
            // We don't want to shoot continuously.
            cam.execute_lua("shoot()", NULL);
            sub_state[SubJoystick::SHOOT] = 1;
        } else if(joy_data[SubJoystick::SHOOT] == 0 && sub_state[SubJoystick::SHOOT] == 1) {
            // Check current state so we're not doing this every loop iteration
            sub_state[SubJoystick::SHOOT] = 0;
        }
        
        // Lights
        if(joy_data[SubJoystick::LIGHTS] == 1 && sub_state[SubJoystick::LIGHTS] == 0) {
            // If the lights aren't already on
            // TODO: Turn the lights on. Which GPIO are we using for this?
        } else if(joy_data[SubJoystick::LIGHTS] == 0 && sub_state[SubJoystick::LIGHTS] == 1) {
            // Check current state so we're not doing this every loop iteration
            // TODO: Turn the lights off
        }
    //}
}
