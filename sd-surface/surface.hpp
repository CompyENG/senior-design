#include <SDL/SDL.h>

bool init();
void clean_up(SDL_Joystick *stick);
void wait_for_ready(SurfaceClient& client,SignalHandler& sigHand);
