
#ifndef LSPASS_SHADOWMAP_H
#define LSPASS_SHADOWMAP_H

#include "lsPass.h"

class lsFBO_Depth;

class lsPass_ShadowMap : public lsPass
{
public:
    lsPass_ShadowMap( int x, int y, int width, int height, lsOpenGL *opengl );
    virtual ~lsPass_ShadowMap();

    void Update();
    void Render();
    void Resize( int x, int y, int width, int height );

    virtual const string& toString();

    lsFBO_Depth* getFBO() const
    {
        return fbo;
    }

private:
    lsFBO_Depth *fbo;
};

#endif // LSPASS_SHADOWMAP_H
