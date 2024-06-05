#include "sdlApp.h"
#include "demoscene.h"


#include <timers/lsTimer.h>


sdlApp::sdlApp()
{
	demoscene = NULL;

	// keys
    key_a = false;
    key_d = false;
    key_w = false;
    key_s = false;
	move_speed = 0.01;

	// Mouse
	rot_speed = 0.05;
	rot_x = 0.0;
	rot_y = 0.0;
}

sdlApp::~sdlApp()
{
}

bool sdlApp::Init()
{
	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << endl;
		return false;

	} else
	{
		// Use OpenGL core - SDL_GL_CONTEXT_DEBUG_FLAG
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG  );
		SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 4 );

		//Create window
		window = SDL_CreateWindow( "vortxGE demo using SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1920, 1080, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
		if( window == NULL )
		{
			cout << "SDL could not Create a window! SDL Error: " << SDL_GetError() << endl;
			return false;

		} else
		{
			//Create context
			context = SDL_GL_CreateContext( window );
			if( context == NULL )
			{
				cout << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << endl;
				return false;

			} else
			{
				SDL_GL_MakeCurrent( window, context );

				// 0 for immediate updates, 1 for updates synchronized with the vertical retrace, -1 for adaptive vsync
				if( SDL_GL_SetSwapInterval( 1 ) < 1 ) // vSync = 1
				{
					cout << "Unable to set VSync method. SDL Error: " << SDL_GetError() << endl;
				}

				demoscene = new DemoScene();
				if(! demoscene  )
				{
					cout << "ERROR: sdlApp::Init() - Can't create demoscene instance." << endl;
					return false;
				} else
				{
					cout << "sdlApp::Init() : demoscene instance created." << endl;
					if( demoscene->Init() )
					{
						cout << "sdlApp::Init() : demoscene->Init() OK." << endl;

					} else
					{
						cout << "ERROR: sdlApp::Init() - demoscene->Init() Failed." << endl;
					}
				}
			}
		}
	}
	return true;
}

//********************
// SDL2 Events

void sdlApp::on_key( SDL_KeyboardEvent* key )
{
	int win_width = 100;
	int win_height = 100;

	switch( key->type )
	{
		case SDL_KEYUP:
			switch( key->keysym.sym )
			{
				case SDLK_a:
					key_a = false;
					break;

				case SDLK_d:
					key_d = false;
					break;

				case SDLK_w:
					key_w = false;
					break;

				case SDLK_s:
					key_s = false;
					break;

				case SDLK_F1:
					// toggle FULLSCREEN
					if( SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN )
					{
						// Free mouse
						SDL_SetRelativeMouseMode( SDL_FALSE );
						SDL_SetWindowFullscreen( window, 0 );

						// Make sure our viewport fits the window
						SDL_GetWindowSize( window, &win_width, &win_height );
						demoscene->Resize( 0, 0, win_width, win_height );

					} else
					{
						SDL_SetWindowFullscreen( window, SDL_WINDOW_FULLSCREEN );
						// Trap mouse
						SDL_SetRelativeMouseMode( SDL_TRUE );

						// Make sure our viewport fits the window
						SDL_GetWindowSize( window, &win_width, &win_height );
						demoscene->Resize( 0, 0, win_width, win_height );

					}
					break;

				// Quit
				case SDLK_ESCAPE:
					running = false;
					break;
			}
			break;

		case SDL_KEYDOWN:
			switch( key->keysym.sym )
			{
				case SDLK_a:
					key_a = true;
					break;

				case SDLK_d:
					key_d = true;
					break;

				case SDLK_w:
					key_w = true;
					break;

				case SDLK_s:
					key_s = true;
					break;
			}
			break;
	}// switch( key->type )
}

void sdlApp::on_mousemove( __attribute__((unused)) int x, __attribute__((unused)) int y, int rx, int ry )
{
	if( SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN )
	{
		rot_x -= (double)rx*rot_speed;
		rot_y += (double)ry*rot_speed;

		demoscene->rotateCamera( rot_x, rot_y, 0.0 );
	}
}

// **************************************
// #################################################

void sdlApp::Run()
{
    // Make sure our viewport fits the window
	int win_width;
	int win_height;
	SDL_GetWindowSize( window, &win_width, &win_height );
	demoscene->Resize( 0, 0, win_width, win_height );

	SDL_Event e;
	int mouseX;
	int mouseY;
	int mouseRelX;
	int mouseRelY;

	// Delta
	double delta = 0.1;
	lsTimer timer;
	timer.start_Timer();

	// Game loop
	running = true;

	while( running )
	{
		//Handle events on queue
		while( SDL_PollEvent( &e ) != 0 )
		{
			switch( e.type )
			{
				case SDL_QUIT:
					running = false;
					break;

				case SDL_KEYDOWN:
				case SDL_KEYUP:
					on_key( &e.key );
					break;

				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
					break;

				case SDL_MOUSEMOTION:
					mouseX = e.motion.x;
					mouseY = e.motion.y;
					mouseRelX = e.motion.xrel;
					mouseRelY = e.motion.yrel;

					on_mousemove( mouseX, mouseY, mouseRelX, mouseRelY );
					break;

				default:
					// cout << "Untracked event : " << e.type << endl;
					break;
			}
		}// while poll event

		if( demoscene )
		{
			// Move camera here so we can combine all key presses.
			demoscene->moveCamera( key_w, key_s, key_a, key_d, delta, move_speed );

			// Update
			demoscene->Update( delta );
			// Render
			SDL_GL_MakeCurrent( window, context );
			demoscene->Render();
		}

		// swap buffers
		SDL_GL_SwapWindow( window );

		// Delta
		delta = timer.get_ElapsedTime();
		timer.start_Timer();

	}// while game loop

	// Release mouse
	SDL_SetRelativeMouseMode( SDL_FALSE );

	// delete our scene
	if( demoscene ) delete demoscene;

	//Destroy window
	if( window )
		SDL_DestroyWindow( window );

	//Quit SDL subsystems
	SDL_Quit();
}

