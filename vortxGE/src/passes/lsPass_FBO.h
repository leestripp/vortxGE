#ifndef vortxGE_lsPass_FBO_h
#define vortxGE_lsPass_FBO_h

#include "lsPass.h"

class lsFBO;

/**
 Pass that renders to FBO
 */
class lsPass_FBO : public lsPass
{
public:
    lsPass_FBO( lsOpenGL* opengl );
    virtual ~lsPass_FBO();

    void Update();
    void Render();
	void Resize( int x, int y, int width, int height  );

    // Save
    virtual const string& toString();


    lsFBO* getFBO() const
    {
        return m_lsfbo;
    }
    void setFBO( lsFBO* val );

private:
    lsFBO *m_lsfbo;

};


#endif
