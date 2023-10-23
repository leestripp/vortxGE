#include <iostream>

#include "glfwApp.h"
#include "demoscene.h"

#include <timers/lsTimer.h>

using namespace std;

// Global object
glfwApp glfw_app;


// *************************************
// GLFW Callbacks

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch( action )
    {
        case GLFW_PRESS:
            switch( key )
            {
                case GLFW_KEY_W:
                    glfw_app.setKey_w( true );
                    break;

                case GLFW_KEY_A:
                    glfw_app.setKey_a( true );
                    break;

                case GLFW_KEY_S:
                    glfw_app.setKey_s( true );
                    break;

                case GLFW_KEY_D:
                    glfw_app.setKey_d( true );
                    break;

                default:
                    break;
            } // switch( key )
            break;

        case GLFW_RELEASE:
            switch( key )
            {
                case GLFW_KEY_W:
                    glfw_app.setKey_w( false );
                    break;

                case GLFW_KEY_A:
                    glfw_app.setKey_a( false );
                    break;

                case GLFW_KEY_S:
                    glfw_app.setKey_s( false );
                    break;

                case GLFW_KEY_D:
                    glfw_app.setKey_d( false );
                    break;

                case GLFW_KEY_ESCAPE:
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                    break;

                default:
                    break;
            } // switch( key )
            break;

        default:
            break;
    } // switch( action )
}

void error_callback( int error, const char* description )
{
    fprintf(stderr, "Error: %s\n", description);
}




// *************************************
// Class start

glfwApp::glfwApp()
{
    demoscene = NULL;

	// keys
    key_a = false;
    key_d = false;
    key_w = false;
    key_s = false;
	move_speed = 0.01;

}

glfwApp::~glfwApp()
{
}

bool glfwApp::Init()
{
    // Setup glfw error callback
    glfwSetErrorCallback( error_callback );

    if(! glfwInit() )
    {
        return false;
    }

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

    // glfwCreateWindow(640, 480, "My Title", NULL, NULL);
    window = glfwCreateWindow( 1920, 1080, "vortxGEdemo_glfw3", NULL, NULL );
    if(! window )
    {
        cout << "glfwCreateWindow - Could not create window..." << endl;
        glfwTerminate();
        return false;
    }

    // Keyboard events
    glfwSetKeyCallback( window, key_callback );

    glfwMakeContextCurrent( window );
    glfwSwapInterval(1); // vSync
    glfwSetWindowPos(window, 256, 256);
    glfwShowWindow( window );

    // Create demo scene
    demoscene = new DemoScene();
    if(! demoscene  )
    {
        cout << "ERROR: glfwApp::Init() - Can't create demoscene instance." << endl;
        return false;
    } else
    {
        cout << "glfwApp::Init() : demoscene instance created." << endl;
        if( demoscene->Init() )
        {
            cout << "glfwApp::Init() : demoscene->Init() OK." << endl;
            int width, height;
            glfwGetFramebufferSize( window, &width, &height );
            demoscene->Resize( 0, 0, width, height );

        } else
        {
            cout << "ERROR: glfwApp::Init() - demoscene->Init() Failed." << endl;
            return false;
        }
    }

    return true;
}


// *********************
// Main LOOP

void glfwApp::Run()
{
	// Delta
	double delta = 0.1;
	lsTimer timer;
	timer.start_Timer();

    running = true;

    while( running )
    {
        if( glfwWindowShouldClose( window ) )
        {
            running = false;
        }

		if( demoscene )
		{
			// Move camera here so we can combine all key presses.
			demoscene->moveCamera( key_w, key_s, key_a, key_d, delta, move_speed );

			// Update
			demoscene->Update( delta );

			// Render
			demoscene->Render();
		}

        // Swap
        glfwSwapBuffers( window );
        // Poll
        glfwPollEvents();

		// Delta
		delta = timer.get_ElapsedTime();
		timer.start_Timer();

    } // while running LOOP

	// delete our scene
	if( demoscene ) delete demoscene;

    glfwDestroyWindow( window );
    glfwTerminate();
}




// ************************
// MAIN

int main( int argc, const char * argv[] )
{
    if( glfw_app.Init() )
    {
        glfw_app.Run();
    }

    return 0;
}
