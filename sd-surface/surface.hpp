#include <SDL/SDL.h>

class SurfaceClient;
class SignalHandler;

bool init();
void clean_up(SDL_Joystick *stick);
void show_connecting_screen(SDL_Surface * screen);
