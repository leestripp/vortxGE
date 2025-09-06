#include <iostream>
// debug
#include "../vortx_debug.h"

#include "../opengl/lsOpenGL.h"

#include "lsFBO.h"


lsFBO::lsFBO( lsOpenGL *opengl )
{
  ogl = opengl;
  fbo_type = LSFT_NONE;

}

lsFBO::~lsFBO()
{
  // cleanup
  if( m_fbo ) glDeleteFramebuffers( 1, &m_fbo );
  // never delete ogl;
}

lsOpenGL *lsFBO::getOgl() const
{
    return ogl;
}

void lsFBO::setOgl(lsOpenGL *value)
{
    ogl = value;
}

