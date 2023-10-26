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

                case GLFW_KEY_F1:
                    if( glfw_app.getFullscreen() )
                    {
                        glfw_app.setFullscreen( false );
                    } else
                    {
                        glfw_app.setFullscreen( true );
                    }
                    glfw_app.CheckFullScreen();
                    break;

                default:
                    break;
            } // switch( key )
            break;

        default:
            break;
    } // switch( action )
}

static void cursor_position_callback( GLFWwindow* window, double xpos, double ypos )
{
    // cout << "Cursor Pos : " <<  xpos << " x " << ypos << endl;

    glfw_app.setPos_x( xpos );
    glfw_app.setPos_y( ypos );
}

static void Resize_Callback( GLFWwindow* window, int width, int height )
{
    glfw_app.setWin_width( width );
    glfw_app.setWin_height( height );
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

    // Mouse - top left is 0x0
    rot_speed = 0.05;
    old_x = 0;
    old_y = 0;
    pos_x = 0;
    pos_y = 0;

    m_fullscreen = false;
    m_win_width_old = 1920;
    m_win_height_old = 1080;
}

glfwApp::~glfwApp()
{
}

void glfwApp::CheckFullScreen()
{
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode( monitor );

    if( m_fullscreen )
    {
        cout << "Go Fullscreen" << endl;

        // Grab mouse
        glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
        // backup window position and window size
        glfwGetWindowSize( window, &m_win_width_old, &m_win_height_old );
        cout << "Store Window Size : " << m_win_width_old << " x " << m_win_height_old << endl;

        glfwWindowHint( GLFW_DECORATED, false );
        glfwSetWindowMonitor( window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate );
        glfwGetWindowSize( window, &m_win_width, &m_win_height );

    } else
    {
        cout << "Go Windowed" << endl;
        cout << "Restore Window Size : " << m_win_width_old << " x " << m_win_height_old << endl;

        // Release mouse
        glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
        glfwWindowHint( GLFW_DECORATED, true );
        glfwSetWindowMonitor( window, NULL, 0, 0, m_win_width_old, m_win_height_old, 0 );
        glfwGetWindowSize( window, &m_win_width, &m_win_height );
    }
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

    glfwWindowHint( GLFW_DECORATED, true );

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
    // Mouse events
    glfwSetCursorPosCallback( window, cursor_position_callback );
    // resize
    glfwSetWindowSizeCallback( window, Resize_Callback );

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
    // Mouse
    double rot_x, rot_y;

    // match window size
    glfwGetWindowSize( window, &m_win_width, &m_win_height );

    running = true;

    while( running )
    {
        if( glfwWindowShouldClose( window ) )
        {
            running = false;
        }

		if( demoscene )
		{
            // Only update camera in fullscreen
            if( m_fullscreen )
            {
                // Rotate
                rot_x -= (pos_x - old_x) *rot_speed;
                rot_y += (pos_y - old_y) *rot_speed;
                demoscene->rotateCamera( rot_x, rot_y, 0.0 );
                old_x = pos_x;
                old_y = pos_y;
                // Move camera here so we can combine all key presses.
                demoscene->moveCamera( key_w, key_s, key_a, key_d, delta, move_speed );
            }

			// Update
			demoscene->Update( delta );

			// Render
            demoscene->Resize( 0, 0, m_win_width, m_win_height );
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

    // Release mouse
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );

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
