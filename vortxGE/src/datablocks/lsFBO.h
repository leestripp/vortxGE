#ifndef LSFBO_H
#define LSFBO_H

// Render
#include "../render/lsRender.h"
#include "../render/lsOpenGL.h"

const int LSFT_NONE             = 0;
const int LSFT_COLOURDEPTH_8	= 1;
const int LSFT_COLOURDEPTH_16F	= 2;
const int LSFT_COLOURDEPTH_32F	= 3;
const int LSFT_DEPTH            = 4;
const int LSFT_DEPTH_16F        = 5;


using namespace std;


class lsFBO
{
public:
    lsFBO( lsRender *render );
    virtual ~lsFBO();

    // Subclass
    virtual void Enable() = 0;
    virtual void ClearBuffers() = 0;

    // get set
    int getFBO_type() const
    {
        return fbo_type;
    }

    void setFBO_type( int val )
    {
        fbo_type = val;
    }

    // dimentions

    void setWidth( double val )
    {
        width = val;
    }

    double getWidth() const
    {
        return width;
    }

    void setHeight( double val )
    {
        height = val;
    }

    double getHeight() const
    {
        return height;
    }

    lsRender *getRender() const;
    void setOgl(lsRender *value);

    // public fbo object
	GLuint m_fbo;

private:
    int fbo_type;
    double width;
    double height;

    // store link
    lsRender *m_render;
};

#endif // LSFBO_H
