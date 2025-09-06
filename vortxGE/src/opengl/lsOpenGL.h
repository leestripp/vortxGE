#ifndef vortxGE_lsOpenGL_h
#define vortxGE_lsOpenGL_h


#include <iostream>
#include <string>

// openGL
#include <GL/glew.h>

class lsStats;
class lsShaderManager;

using namespace std;

class lsOpenGL
{
public:
    lsOpenGL();
    virtual ~lsOpenGL();

    void Init();
    void glInfo();
    void rendererChanged();

    void CheckGLErrors( const string &name );

    // Store FBO
    void StoreFBO();
    void RestoreFBO();

    lsStats* getStats() const
    {
        return stats;
    }

    void setStats( lsStats* val )
    {
        stats = val;
    }

    lsShaderManager* getShaderManager() const
    {
        return shadermanager;
    }

    void setShaderManager( lsShaderManager* val )
    {
        shadermanager = val;
    }

    // Features ********

    bool hasVBO() const
    {
        return m_hasVBO;
    }

    bool hasFBO() const
    {
        return m_hasFBO;
    }

    bool hasShaders() const
    {
        return m_hasShaders;
    }

private:
    lsStats             *stats;
    lsShaderManager     *shadermanager;

    // Store FBO
    GLint default_FBO;
    // Features
    bool m_hasVBO;
    bool m_hasFBO;
    bool m_hasShaders;
};

#endif
