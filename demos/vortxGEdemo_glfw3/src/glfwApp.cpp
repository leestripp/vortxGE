/**
 * @file glfwApp.cpp
 * @brief GLFW application implementation
 * 
 * This file implements the GLFW-based demo application, handling:
 * - Window creation and event processing
 * - Input callback registration
 * - Main render loop
 * - Application lifecycle (init, run, cleanup)
 * 
 * @see glfwApp.h
 */

#include <iostream>
#include <ctime>

#include "glfwApp.h"

// vortxGE engine header — PreferredRendererType() for backend selection
#include "vortxGE.h"
#include "render/lsRender.h" // lsRender::Screenshot (builtin screenshot, F12)
#include "demoscene.h"

#include <timers/lsTimer.h>

using namespace std;

// Global object
glfwApp glfw_app;


// *************************************
// GLFW Callbacks

static void key_callback(GLFWwindow* window, int key, __attribute__((unused)) int scancode, int action, int /* mods */)
{
	bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);

	if (action == GLFW_RELEASE)
	{
		switch (key) {
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(window, GLFW_TRUE);
				break;
			
			case GLFW_KEY_F1:
				glfw_app.setFullscreen(!glfw_app.getFullscreen());
				glfw_app.CheckFullScreen();
				break;

			case GLFW_KEY_F12:
				glfw_app.requestScreenshot();
				break;
		}
	}
	
	// Handle movement keys
	switch (key)
	{
		case GLFW_KEY_W: glfw_app.setKey_w(pressed); break;
		case GLFW_KEY_A: glfw_app.setKey_a(pressed); break;
		case GLFW_KEY_S: glfw_app.setKey_s(pressed); break;
		case GLFW_KEY_D: glfw_app.setKey_d(pressed); break;
		case GLFW_KEY_SPACE: glfw_app.setKey_jump(action == GLFW_PRESS); break; // Jump on press only
		case GLFW_KEY_LEFT_SHIFT: glfw_app.setKey_sprint(pressed); break;
		case GLFW_KEY_LEFT_CONTROL: glfw_app.setKey_crouch(pressed); break;
	}
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int /* mods */)
{
    if (!window)
    {
        cerr << "ERROR: mouse_button_callback() called with NULL window." << endl;
        return;
    }

    // Reset the buttons to false by default
    glfw_app.setMBL(false);
    glfw_app.setMBR(false);
	
	switch(action)
	{
		case GLFW_PRESS:
			if (button == GLFW_MOUSE_BUTTON_LEFT)
   			{
   				glfw_app.setMBL(true);
   			}
			if (button == GLFW_MOUSE_BUTTON_RIGHT)
   			{
   				glfw_app.setMBR(true);
   			}
			break;
		
		case GLFW_RELEASE:
			if (button == GLFW_MOUSE_BUTTON_LEFT)
   			{
   				glfw_app.setMBL(false);
   			}
			if (button == GLFW_MOUSE_BUTTON_RIGHT)
   			{
   				glfw_app.setMBR(false);
   			}
			break;
	}
}

static void cursor_position_callback( __attribute__((unused)) GLFWwindow* window, double xpos, double ypos )
{
	glfw_app.setPos_x( xpos );
	glfw_app.setPos_y( ypos );
}

// Called when the framebuffer pixel size changes (i.e. when the OS
// actually allocates a new buffer) — typically from window resize, but
// on Wayland also from monitor-scale-factor changes. The (width, height)
// here are PHYSICAL pixels, not logical window units.
//
// We forward the new size to the demo scene so the main FBO and all
// passes get re-sized correctly. Without this, dragging the window to
// a HiDPI monitor, or changing the desktop scale factor at runtime,
// would leave the previous FBO size mismatched with the new framebuffer.
static void Resize_Callback( __attribute__((unused)) GLFWwindow* window, int width, int height )
{
	glfw_app.setWin_width( width );
	glfw_app.setWin_height( height );

	if( glfw_app.getDemoscene() )
	{
		glfw_app.getDemoscene()->Resize( 0, 0, width, height );
	}
}

void error_callback( int error, const char* description )
{
	cerr << "ERROR: " << description << ", code=" << error << endl;
}


// *************************************
// Class start

glfwApp::glfwApp()
{
	demoscene = nullptr;
	m_window = nullptr;
	
	// keys
	key_w = key_a = key_s = key_d = false;
	key_jump = key_sprint = key_crouch = false;
	
	// Mouse - top left is 0x0
	old_x = 0;
	old_y = 0;
	pos_x = 0;
	pos_y = 0;
	m_mouse_look_active = false;
	
	m_fullscreen = false;
	m_win_width_old = 1920;
	m_win_height_old = 1080;
}

glfwApp::~glfwApp()
{
}


void glfwApp::CheckFullScreen()
{
	if (! m_window) {
		cerr << "ERROR: CheckFullScreen() called with NULL m_window." << endl;
		return;
	}
	
	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode *mode = glfwGetVideoMode( monitor );

	if( m_fullscreen )
	{
		cout << "Go Fullscreen" << endl;

		// Grab mouse
		glfwSetInputMode( m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
		// backup window position and window size
		glfwGetWindowSize( m_window, &m_win_width_old, &m_win_height_old );
		cout << "Store Window Size : " << m_win_width_old << " x " << m_win_height_old << endl;

		glfwWindowHint( GLFW_DECORATED, false );
		glfwSetWindowMonitor( m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate );
		// Query framebuffer in physical pixels (not logical window units).
		// On HiDPI Wayland with 125% scaling, the framebuffer is
		// 1.25x the requested monitor-mode size.
		glfwGetFramebufferSize( m_window, &m_win_width, &m_win_height );

		// Propagate immediately so the next frame uses the new size
		// rather than waiting for the callback to fire (which it
		// also does — glfwSetWindowMonitor schedules a resize callback
		// on the next event-loop iteration). The callback might run
		// AFTER the first fullscreen frame is already drawn, leaving
		// one frame of misaligned FBO. Force-propagating now keeps
		// the fullscreen transition seamless.
		if( glfw_app.getDemoscene() )
		{
			glfw_app.getDemoscene()->Resize( 0, 0, m_win_width, m_win_height );
		}

	} else
	{
		cout << "Go Windowed" << endl;
		cout << "Restore Window Size : " << m_win_width_old << " x " << m_win_height_old << endl;

		// Release mouse
		glfwSetInputMode( m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
		glfwWindowHint( GLFW_DECORATED, true );
		glfwSetWindowMonitor( m_window, NULL, 0, 0, m_win_width_old, m_win_height_old, 0 );
		glfwGetFramebufferSize( m_window, &m_win_width, &m_win_height );

		// Force-propagate so the next frame matches the new window
		// dimensions (see the matching comment in the Go Fullscreen
		// branch above for the rationale).
		if( glfw_app.getDemoscene() )
		{
			glfw_app.getDemoscene()->Resize( 0, 0, m_win_width, m_win_height );
		}
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

	// Decide the backend BEFORE creating the window: Vulkan needs a
	// GLFW_NO_API window (no GL context), OpenGL needs GL context hints.
	m_renderType = vortxGE::PreferredRendererType();
	if( m_renderType == RenderType::Vulkan )
	{
		glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
		cout << "glfwApp::Init() - creating window for Vulkan (no GL context)" << endl;
	} else
	{
		glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
		glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
		glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	}

	glfwWindowHint( GLFW_DECORATED, true );

	// HiDPI / fractional-scale fix (Wayland + macOS):
	// GLFW_SCALE_FRAMEBUFFER was added in GLFW 3.4 so that when the user
	// has 125%/150% desktop scaling, the OS allocates a framebuffer at
	// logical-size * scale rather than just logical-size. On older GLFW
	// or buggy Wayland compositors this is a no-op, but it costs nothing
	// to set. We still apply the content scale manually below as a
	// belt-and-braces fallback.
	glfwWindowHint( GLFW_SCALE_FRAMEBUFFER, GLFW_TRUE );

	// glfwCreateWindow(640, 480, "My Title", NULL, NULL);
	m_window = glfwCreateWindow( 1920, 1080, "vortxGEdemo_glfw3", NULL, NULL );
	if(! m_window )
	{
		cout << "ERROR: glfwCreateWindow - Could not create window..." << endl;
		glfwTerminate();
		return false;
	}

	// Keyboard events
	glfwSetKeyCallback( m_window, key_callback );
	// Mouse events
	glfwSetCursorPosCallback(m_window, cursor_position_callback );
	glfwSetMouseButtonCallback(m_window, mouse_button_callback); // Add mouse button callback
	// resize
// Note: set FRAMEBUFFER size callback (not window) so the callback
	// receives physical pixels regardless of HiDPI scaling.
	glfwSetFramebufferSizeCallback( m_window, Resize_Callback );

	if( m_renderType == RenderType::OpenGL )
	{
		glfwMakeContextCurrent( m_window );
		glfwSwapInterval( 1 ); // vSync (Vulkan: FIFO present mode in the backend)
	}
	glfwSetWindowPos( m_window, 256, 256 );
	glfwShowWindow( m_window );
	
	// Create demo scene — hand it the backend choice + window so the
	// engine initialises the matching renderer.
	demoscene = new DemoScene();
	demoscene->SetRendererPreference( m_renderType, m_window );
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
			// Compute the TRUE physical pixel size of the framebuffer.
			//
			// On Wayland with HiDPI / fractional desktop scaling
			// (e.g. 125%/150%), glfwGetFramebufferSize() historically
			// returned only the logical window size — i.e. it didn't
			// apply the compositor's content scale factor. (GLFW issue
			// #1168 is the canonical report.) Multiplied by the scale
			// read from glfwGetWindowContentScale(), we always get
			// the physical pixels the OS actually allocated, regardless
			// of whether the GLFW_SCALE_FRAMEBUFFER hint is honoured.
			glfwGetFramebufferSize( m_window, &width, &height );
			float xscale = 1.0f, yscale = 1.0f;
			glfwGetWindowContentScale( m_window, &xscale, &yscale );

			// Some Wayland builds (and XWayland) report scale=0.0 when no
			// scaling is in effect; the validity check below treats that
			// as "no extra scaling needed".
			if( xscale > 0.0f && yscale > 0.0f &&
			    !( xscale == 1.0f && yscale == 1.0f ) )
			{
				width  = (int)( width  * xscale );
				height = (int)( height * yscale );
			}

#ifdef VORTX_DEBUG_OPENGLERRORS
			cout << "glfwApp::Init viewport size: "
			     << width << " x " << height
			     << " (logical content scale "
			     << xscale << " x " << yscale << ")"
			     << endl;
#endif
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
	double delta = 1000.0 / 60.0; // Sensible default for first frame (approx 16.67ms)
	lsTimer timer;
	timer.start_Timer();
	
	// match window size
	glfwGetWindowSize( m_window, &m_win_width, &m_win_height );
	
	running = true;

	while( running )
	{
		if( glfwWindowShouldClose( m_window ) )
		{
			running = false;
		}

		if( demoscene )
		{
			// Only update camera in fullscreen mode
			if( m_fullscreen )
			{
				if (!m_mouse_look_active)
				{
					// First frame in mouse look mode, sync positions to prevent jump
					glfwGetCursorPos(m_window, &old_x, &old_y);
					pos_x = old_x;
					pos_y = old_y;
					m_mouse_look_active = true;
				}

				// Rotate
				demoscene->rotatePlayerView( (pos_x - old_x), (pos_y - old_y) );
				old_x = pos_x;
				old_y = pos_y;

				// Move player
				demoscene->movePlayer( key_w, key_s, key_a, key_d, key_jump, key_sprint, key_crouch );
				
				// Handle mouse buttons
				demoscene->handleMouseButtons(m_mouse_button_left, m_mouse_button_right);
				
				if (key_jump) setKey_jump(false); // Reset jump state after processing
			} else
			{
				// Not in fullscreen, so mouse look is inactive
				m_mouse_look_active = false;
			}

			// Update
			demoscene->Update( delta );

			// Render
			demoscene->Resize( 0, 0, m_win_width, m_win_height );
			demoscene->Render();
		}

		// Builtin screenshot (F12): take the requested capture AFTER the
		// frame rendered — for GL the back buffer is still intact here
		// (pre-swap); Vulkan reads back the last presented swapchain image.
		if( m_screenshotRequested )
		{
			m_screenshotRequested = false;
			time_t t = time( nullptr );
			char name[128];
			strftime( name, sizeof(name),
					  "screenshot_%Y%m%d_%H%M%S.png", localtime( &t ) );
			if( demoscene && demoscene->getEngine() &&
				demoscene->getEngine()->getRender() )
				demoscene->getEngine()->getRender()->Screenshot( name );
		}

		// Swap (GL only — the Vulkan backend presents in lsVulkan::Present
		// via vortxGE::Render, and glfwSwapBuffers has no GL context here)
		if( m_renderType == RenderType::OpenGL )
			glfwSwapBuffers( m_window );
		// Poll
		glfwPollEvents();

		// Delta
		delta = timer.get_ElapsedTime();
		timer.start_Timer();

	} // while running LOOP

	// Release mouse
	glfwSetInputMode( m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );

	// delete our scene
	if( demoscene ) delete demoscene;
	// window
	if(m_window) glfwDestroyWindow( m_window );
	
	glfwTerminate();
}

//**********************
// MAIN: Do not mode as it uses the global alfw_app variable.

int main()
{
	if(! glfw_app.Init())
	{
		return -1;
	}
	
	glfw_app.Run();
	
	return 0;
}
