#include <SDL/SDL.h>

class SurfaceClient;
class SignalHandler;

bool init();
void clean_up(SDL_Joystick *stick);
void show_image_status(const char * image, SDL_Surface * screen);
