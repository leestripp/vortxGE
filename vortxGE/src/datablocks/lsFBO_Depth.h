#ifndef LSFBO_DEPTH_H
#define LSFBO_DEPTH_H

#include "../render/lsOpenGL.h"

#include "lsFBO.h"

class lsFBO_Depth : public lsFBO
{
public:
    lsFBO_Depth( double width, double height, lsOpenGL *opengl );
    virtual ~lsFBO_Depth();

    // From lsFBO
    void Enable();
    void ClearBuffers();

    GLuint getDepthTexture()
    {
        return depth_texture;
    }

private:
    GLuint depth_texture;
};

#endif // LSFBO_DEPTH_H
