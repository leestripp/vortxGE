#ifndef SDLAPP_H
#define SDLAPP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>

class DemoScene;


class sdlApp
{
public:
	sdlApp();
	virtual ~sdlApp();

	bool Init();
	void Run();

protected:
	void on_key( SDL_KeyboardEvent *key );
	void on_mousemove( int x, int y, int rx, int ry );

private:
	DemoScene		*demoscene;

    SDL_Window		*window;
    SDL_GLContext	context;

    bool running;
	// keys
    bool key_a;
    bool key_d;
    bool key_w;
    bool key_s;
    // Mouse
    double rot_speed;
    double move_speed;
    double rot_x;
    double rot_y;
};

#endif // SDLAPP_H
