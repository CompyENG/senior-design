#include <signal.h>
#include <errno.h>

#include "SignalHandler.hpp"

// Code borrowed from http://www.yolinux.com/TUTORIALS/C++Signals.html

bool SignalHandler::mbGotExitSignal = false;

/**
* Default Contructor.
*/
SignalHandler::SignalHandler() {
    
}

/**
* Destructor.
*/
SignalHandler::~SignalHandler() {
    
}

/**
* Returns the bool flag indicating whether we received an exit signal
* @return Flag indicating shutdown of program
*/
bool SignalHandler::gotExitSignal() {
    return mbGotExitSignal;
}

/**
* Sets the bool flag indicating whether we received an exit signal
*/
void SignalHandler::setExitSignal(bool _bExitSignal) {
    mbGotExitSignal = _bExitSignal;
}

/**
* Sets exit signal to true.
* @param[in] _ignored Not used but required by function prototype
*                     to match required handler.
*/
void SignalHandler::exitSignalHandler(int _ignored) {
    mbGotExitSignal = true;
}

bool SignalHandler::gotUpdateSignal() {
    return mbGotUpdateSignal;
}

void SignalHandler::setUpdateSignal(bool _bUpdateSignal) {
    mbGotUpdateSignal = _bUpdateSignal;
}

void SignalHandler::updateSignalHandler(int _ignored) {
    mbGotUpdateSignal = true;
}

/**
* Set up the signal handlers for CTRL-C.
*/
void SignalHandler::setupSignalHandlers() {
    if (signal((int) SIGINT, SignalHandler::exitSignalHandler) == SIG_ERR)
    {
        throw 1;
    }
    
    if (signal((int) SIGUSR1, SignalHandler::updateSignalHandler) == SIG_ERR)
    {
        throw 1;
    }
}

