#ifndef SDLAPP_H
#define SDLAPP_H

#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

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
    bool key_w, key_a, key_s, key_d;
    bool key_jump, key_sprint, key_crouch;
};

#endif // SDLAPP_H
