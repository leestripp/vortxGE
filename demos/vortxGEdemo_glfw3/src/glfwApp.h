#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class DemoScene;

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

    void setKey_w( bool val ) { key_w = val; }
    void setKey_a( bool val ) { key_a = val; }
    void setKey_s( bool val ) { key_s = val; }
    void setKey_d( bool val ) { key_d = val; }
    void setKey_jump( bool val ) { key_jump = val; }
    void setKey_sprint( bool val ) { key_sprint = val; }
    void setKey_crouch( bool val ) { key_crouch = val; }

    // Mouse
    void setPos_x( double val ) { pos_x = val; }
    void setPos_y( double val ) { pos_y = val; }

    // resize window
    void setWin_width( int val ) { m_win_width = val; }
    void setWin_height( int val ) { m_win_height = val; }

    // Fullscreen
    bool getFullscreen() { return m_fullscreen; }
    void setFullscreen( bool val ) { m_fullscreen = val; }

private:
	DemoScene		*demoscene;
    GLFWwindow      *window;

    bool running;

    // window
    bool m_fullscreen;
    int m_win_width;
    int m_win_height;
    int m_win_width_old;
    int m_win_height_old;

	// keys
    bool key_w, key_a, key_s, key_d;
    bool key_jump, key_sprint, key_crouch;

    // Mouse
    double old_x, old_y;
    double pos_x, pos_y;
};

