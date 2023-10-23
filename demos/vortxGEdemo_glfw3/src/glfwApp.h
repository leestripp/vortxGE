#ifndef GLFWAPP_H
#define GLFWAPP_H

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

    bool getRunning()
    {
        return running;
    }

    void setKey_w( bool val )
    {
        key_w = val;
    }

    void setKey_a( bool val )
    {
        key_a = val;
    }

    void setKey_s( bool val )
    {
        key_s = val;
    }

    void setKey_d( bool val )
    {
        key_d = val;
    }

private:
	DemoScene		*demoscene;
    GLFWwindow      *window;

    bool running;
	// keys
    bool key_a;
    bool key_d;
    bool key_w;
    bool key_s;
    double move_speed;

    // Mouse
    double rot_speed;
    double rot_x;
    double rot_y;
};

#endif
