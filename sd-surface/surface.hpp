#include <SDL/SDL.h>

class SurfaceClient;
class SignalHandler;

bool init();
void resizeImage( SDL_Surface*& img, const double newwidth, const double newheight );
void matchColorKeys( SDL_Surface* src, SDL_Surface* dest );
Uint32 get_pixel( SDL_Surface* surface, int x, int y );
void put_pixel( SDL_Surface *surface, int x, int y, Uint32 pixel );
void clean_up(SDL_Joystick *stick);

