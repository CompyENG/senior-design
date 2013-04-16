#include <SDL/SDL.h>

class SurfaceClient;
class SignalHandler;

bool init();
void clean_up(SDL_Joystick *stick);
void show_image_status(const char * image, SDL_Surface * screen);
void draw_bmp_location(const char * image_path, SDL_Surface * screen, int x, int y);
