/**
 * @file glfwApp.h
 * @brief GLFW-based demo application
 * 
 * This class wraps the GLFW window management library and provides
 * a complete demo application framework for vortxGE.
 * 
 * Features:
 * - GLFW window creation and management
 * - OpenGL context initialization via GLEW
 * - Keyboard input handling (WASD, Space, Shift, Ctrl)
 * - Mouse input (movement, buttons)
 * - Fullscreen toggle (F1)
 * - Main game loop with timing
 * 
 * Key bindings:
 * - WASD: Movement
 * - Space: Jump
 * - Shift: Sprint
 * - Ctrl: Crouch
 * - F1: Toggle fullscreen
 * - ESC: Exit
 * 
 * @see glfwApp.cpp
 * @see DemoScene
 */

#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class DemoScene;

#include "render/lsRenderTypes.h"

class glfwApp
{
public:
	glfwApp();
	~glfwApp();

	bool Init();
	void Run();
	void CheckFullScreen();

	bool getRunning()
	{
		return running;
	}

	// Key state setters
	void setKey_w( bool val ) { key_w = val; }
	void setKey_a( bool val ) { key_a = val; }
	void setKey_s( bool val ) { key_s = val; }
	void setKey_d( bool val ) { key_d = val; }
	void setKey_jump( bool val ) { key_jump = val; }
	void setKey_sprint( bool val ) { key_sprint = val; }
	void setKey_crouch( bool val ) { key_crouch = val; }

	// Mouse position setters
	void setPos_x( double val ) { pos_x = val; }
	void setPos_y( double val ) { pos_y = val; }
	// Mouse buttons
	void setMBL(bool val) { m_mouse_button_left = val; }
	void setMBR(bool val) { m_mouse_button_right = val; }
	
	// Resize window setters
	void setWin_width( int val ) { m_win_width = val; }
	void setWin_height( int val ) { m_win_height = val; }

	// Fullscreen getters/setters
	bool getFullscreen() { return m_fullscreen; }
	void setFullscreen( bool val ) { m_fullscreen = val; }
	
	// Get demoscene
	DemoScene* getDemoscene() { return demoscene; }

	/// Builtin screenshot (F12): request a capture; the Run loop takes it
	/// after the next Render so both backends are in a readable state.
	void requestScreenshot() { m_screenshotRequested = true; }

private:
	bool m_screenshotRequested = false;
	DemoScene* demoscene;
	GLFWwindow* m_window;
	RenderType  m_renderType = RenderType::None;   // backend chosen at window creation
	
	bool running;

	// Window dimensions
	bool m_fullscreen;
	int m_win_width;
	int m_win_height;
	int m_win_width_old;
	int m_win_height_old;

	// Key states
	bool key_w, key_a, key_s, key_d;
	bool key_jump, key_sprint, key_crouch;

	// Mouse position
	double old_x, old_y;
	double pos_x, pos_y;
	bool m_mouse_look_active;
	
	// buttons
	bool m_mouse_button_left = false;
	bool m_mouse_button_right = false;
};
