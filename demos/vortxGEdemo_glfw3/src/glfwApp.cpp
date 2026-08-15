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
#include <chrono>
#include <cstring>

#include "glfwApp.h"

// vortxGE engine header — PreferredRendererType() for backend selection
#include "vortxGE.h"
#include "render/lsRender.h" // lsRender::Screenshot (builtin screenshot, F12)
#include "debug/lsDebug.h" // VORTX_DEBUG_TOOLS gate
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

			case GLFW_KEY_F2:
				// player rig (first-person, physics) — explicit switch
				if( glfw_app.getDemoscene() )
					glfw_app.getDemoscene()->setFlyCam( false );
				break;

			case GLFW_KEY_F3:
				// flycam (noclip survey camera) — explicit switch
				if( glfw_app.getDemoscene() )
					glfw_app.getDemoscene()->setFlyCam( true );
				break;

			case GLFW_KEY_F6:
				// noclip-style survey camera toggle (rig <-> flycam)
				if( glfw_app.getDemoscene() )
					glfw_app.getDemoscene()->toggleFlyCam();
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

	// GLFW_PLATFORM: this distro's GLFW 3.4 ignores the GLFW_PLATFORM env
	// var (probe: always Wayland), so apply the platform hint ourselves.
	// Needed for RenderDoc captures (no Wayland support) via
	// GLFW_PLATFORM=x11 DISPLAY=:99 (XWayland).
	if( const char* plat = getenv( "GLFW_PLATFORM" ) )
	{
		if( strcmp( plat, "x11" ) == 0 )
			glfwInitHint( GLFW_PLATFORM, GLFW_PLATFORM_X11 );
		else if( strcmp( plat, "wayland" ) == 0 )
			glfwInitHint( GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND );
	}

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

	static const bool phaseTime = std::getenv( "VORTX_PHASE_TIME" ) != nullptr;
	static double accU = 0, accR = 0, accS = 0; static int accN = 0;
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

			auto tp0 = std::chrono::steady_clock::now();
			// Update
			demoscene->Update( delta );

			// FIX 2026-08-11 (startup black bands — see Plan.md "D1").
			// Self-healing size poll. The framebuffer's PHYSICAL size can
			// change without a Resize_Callback: at startup the Wayland
			// compositor applies the monitor scale factor only when the
			// window is first mapped (Init queried too early and stored
			// the logical size), and compositor-side scale changes do
			// not always fire the callback either. The callback path
			// delivers physical pixels, so the raw getter (no content-
			// scale multiplication — that would double-apply once the
			// getter returns physical values) is the source of truth.
			// FBO recreation only happens when the size actually changed.
			{
				int fbw = 0, fbh = 0;
				glfwGetFramebufferSize( m_window, &fbw, &fbh );
				if( fbw > 0 && fbh > 0 &&
					( fbw != m_win_width || fbh != m_win_height ) )
				{
					m_win_width = fbw;
					m_win_height = fbh;
				}
			}

			// Render
			demoscene->Resize( 0, 0, m_win_width, m_win_height );
			auto tp1 = std::chrono::steady_clock::now();
			demoscene->Render();
			if( phaseTime )
			{
				auto tp2 = std::chrono::steady_clock::now();
				accU += std::chrono::duration<double,std::milli>( tp1 - tp0 ).count();
				accR += std::chrono::duration<double,std::milli>( tp2 - tp1 ).count();
				accN++;
			}
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
		if( phaseTime )
		{
			auto tp3 = std::chrono::steady_clock::now();
			glfwPollEvents();
			auto tp4 = std::chrono::steady_clock::now();
			accS += std::chrono::duration<double,std::milli>( tp4 - tp3 ).count();
			if( accN >= 120 )
			{
				printf( "PHASE update=%.2fms render=%.2fms poll=%.2fms (n=%d)\n",
						accU / accN, accR / accN, accS / accN, accN );
				accU = accR = accS = 0; accN = 0;
			}
		}
		else
		{
			// Poll
			glfwPollEvents();
		}

		// Delta
		delta = timer.get_ElapsedTime();
		timer.start_Timer();
#if defined(VORTX_DEBUG_TOOLS)
		// D2 diagnostic: VORTX_FIXED_DT=ms forces a fixed timestep so
		// frame-N state dumps (VORTX_FRAME_DUMP) are deterministic
		// across runs — required for GL/VK diffing of an animated,
		// physics-driven scene.
		static double s_fixedDt = -1.0;
		if( s_fixedDt < 0.0 )
		{
			const char* e = getenv( "VORTX_FIXED_DT" );
			s_fixedDt = ( e && *e ) ? atof( e ) : 0.0;
		}
		if( s_fixedDt > 0.0 ) delta = s_fixedDt;
#endif

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
