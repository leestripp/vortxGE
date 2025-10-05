#include <iostream>
// debug
#include "../vortx_debug.h"

#include "lsFBO.h"


lsFBO::lsFBO( lsRender *render )
{
  m_render = render;
  fbo_type = LSFT_NONE;

}

lsFBO::~lsFBO()
{
  // cleanup
  if( m_fbo ) glDeleteFramebuffers( 1, &m_fbo );
  // never delete m_render;
}

lsRender *lsFBO::getRender() const
{
    return m_render;
}

void lsFBO::setOgl(lsRender *value)
{
    m_render = value;
}

