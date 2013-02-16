#include <bcm2835.h>

#include "Motor.hpp"

bool Motor::gpio_setup = false;

Motor::Motor(int * GPIO) {
    // Set up GPIO (if it hasn't been already)
    this->setup_gpio();
    // Set up the pins we received
    this->setup(GPIO);
}

Motor::Motor() {
    // Set up GPIO (if it hasn't been already)
    this->setup_gpio();
}

bool Motor::setup_gpio() {
    // This function needs to be called once per application
    if(!Motor::gpio_setup) {
        if (!bcm2835_init()) {
            return false;
        }
        Motor::gpio_setup = true;
    }
    return true;
}

bool Motor::setup(int * GPIO) {
    if(!Motor::gpio_setup) return false;
    
    pins[0] = *GPIO;
    pins[1] = *(GPIO+1);
    
    bcm2835_gpio_fsel(pins[0], BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(pins[1], BCM2835_GPIO_FSEL_OUTP);
    
    return true;
}

bool Motor::spin(int direction) {
    switch(direction) {
        case Motor::FORWARD:
            return this->spinForward();
            break;
        case Motor::BACKWARD:
        case Motor::REVERSE:
            return this->spinReverse();
            break;
        case Motor::OFF:
            return this->stop();
            break;
        default:
            return this->stop(); // If we don't understand the command, stop the motor
            break;
    }
    // If we made it here, something went wrong...
    return false;
}

bool Motor::spinForward() {
    if(!Motor::gpio_setup) return false;
    
    bcm2835_gpio_write(pins[0], LOW);
    bcm2835_gpio_write(pins[1], HIGH);
    // It seems the bcm2835 library doesn't let us detect if this works.  This
    //  class was designed with bools in the hopes that I could. Oh well... :/
    return true;
}

bool Motor::spinReverse() {
    if(!Motor::gpio_setup) return false;
    
    bcm2835_gpio_write(pins[0], LOW); // Always set low first
    bcm2835_gpio_write(pins[1], HIGH);
    return true;
}

bool Motor::spinBackward() {
    return this->spinReverse();
}

bool Motor::stop() {
    if(!Motor::gpio_setup) return false;
    
    bcm2835_gpio_write(pins[0], LOW);
    bcm2835_gpio_write(pins[1], LOW);
    
    return true;
}
    
