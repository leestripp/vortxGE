#include <iostream>
// debug
#include "../vortx_debug.h"

// OpenGL
// TODO(Phase 8 cleanup): replace GL_* constants with engine-level
// constants (LS_TRUE, LS_DEPTH_TEST, etc.). Until then this file still
// uses engine-level LS_* constants (via lsRenderConstants.h, transitively through lsRender.h).
#include "../render/lsRender.h"
#include "../render/lsRenderConstants.h"

// datablocks
#include "../datablocks/lsStats.h"
#include "../datablocks/lsFBO.h"

// Scene
#include "../scene/lsScene.h"
#include "../scene/lsCamera.h"

#include "lsPass_FBO.h"

lsPass_FBO::lsPass_FBO( lsRender* render ) : lsPass(render)
{
    setPass_type( LSPT_FBO );
    setName("Pass : FBO");

    m_lsfbo = NULL;

}

lsPass_FBO::~lsPass_FBO()
{
    // fbo's are not assets, so must cleanup
    if( m_lsfbo ) delete m_lsfbo;
}

/**
 Make sure our scene is ready for drawing
 */
void lsPass_FBO::Update()
{
#ifdef VORTX_DEBUG_RENDER
    cout << "lsPass_FBO::Update" << endl;
#endif

    // Init our scene
    if( getScene()  ) getScene()->InitDrawlist();

    if(( getScene() )&&( getCull_camera() ))
    {
        if( getPrev_pass() )
        {
            if( getPrev_pass()->getCull_camera() )
            {
                // Optimize if view change from last pass
                if( getPrev_pass()->getCull_camera() != getCull_camera() )
                {
                    // Optimize stats
                    if( getRender()->getStats() ) getRender()->getStats()->Optimize_Start();
                    // Optimize Lights first
                    getScene()->Optimize(getCull_camera(), this, getScene(), getRender());
                    // Scene drawables.
                    getScene()->OptimizeThread( getCull_camera(), this, getScene(), getRender() );
                    if( getRender()->getStats() ) getRender()->getStats()->Optimize_Stop();
                }
            }
        } else
        {
            // Optimize stats
            if( getRender()->getStats() ) getRender()->getStats()->Optimize_Start();
            getScene()->OptimizeThread( getCull_camera(), this, getScene(), getRender() );
            if( getRender()->getStats() ) getRender()->getStats()->Optimize_Stop();
        }
    }// if scene
}


void lsPass_FBO::Render()
{
#ifdef VORTX_DEBUG_RENDER
    cout << "lsPass_FBO::Render" << endl;
#endif

    if( m_lsfbo )
    {
        // Store current FBO
        getRender()->StoreDefaultFramebuffer();
		
        // use FBO
        m_lsfbo->Enable();
		
		// set viewport
		getRender()->Viewport( getPosition().x, getPosition().y, getSize().x, getSize().y );

        getRender()->ClearColor( getBackground_colour().x, getBackground_colour().y, getBackground_colour().z, getBackground_colour().w );
        getRender()->ClearDepth( 1.0 );
        getRender()->Enable( LS_DEPTH_TEST );
        getRender()->DepthMask( LS_TRUE );
		// default depth range.
		getRender()->DepthRangef( 0.0f, 1.0f );
		
        m_lsfbo->ClearBuffers();
		
        // Draw
        if( getScene() ) getScene()->Draw( getCamera(), this, NULL, getRender() );

        // Make sure we are on the default framebuffer
        getRender()->RestoreDefaultFramebuffer();

    } else
    {
        cout << "ERROR: lsPass_FBO::Render - lsFBO object not set" << endl;
    }// if fbo
}

void lsPass_FBO::Resize( int x, int y, int width, int height  )
{
	float rw, rh;
	rw = width-x;
	rh = height - y;
	setVratio( rw / rh );

	// Update the FBO pass viewport to match the new framebuffer pixel
	// size. Previously this code path was commented out as a TODO, which
	// meant the FBO stayed at its construction size (e.g. 1920×1080) and
	// the FBO texture covered only the top-left of the window when the
	// framebuffer was larger (HiDPI / display scaling / window resize).
	//
	// If we have an FBO attached, recreate its GL resources at the new
	// pixel dimensions, then update the pass viewport to match.
	if( m_lsfbo && ( (double)width != m_lsfbo->getWidth() || (double)height != m_lsfbo->getHeight() ) )
	{
		m_lsfbo->Resize( (double)width, (double)height );
	}
	setPosition( lsVector2( x, y ) );
	setSize( lsVector2( width, height ) );

    // update camera
    if( getCamera() )
    {
        if( getCamera()->getCamera_mode() == LSCM_PERSPECTIVE )
        {
            getCamera()->Set_ViewRatio( getVratio() );
        }
    }

    if( getCull_camera() )
    {
        if(( getCull_camera()->getCamera_mode() == LSCM_PERSPECTIVE )&&( getCamera() != getCull_camera() ))
        {
            getCull_camera()->Set_ViewRatio( getVratio() );
        }
    }

#ifdef VORTX_DEBUG_RENDER
    cout << "lsPass_FBO::Resize - ";
    cout << "viewport Size: " << getSize().x << " x " << getSize().y << " vRatio: " << getVratio() << endl;
#endif
}

void lsPass_FBO::setFBO( lsFBO* val )
{
    m_lsfbo = val;
    setSize( lsVector2( m_lsfbo->getWidth(), m_lsfbo->getHeight() ) );
    setVratio(1.0);
}


// ####################
// Save data to disk

const string& lsPass_FBO::toString()
{
    string str;
    string tmp;

    str = "add_pass FBO ";
    if( getScene() )
    {
        tmp = getScene()->getName();
        tmp.replace( tmp.begin(), tmp.end(), ' ', '#');
        str += tmp;
    } else
    {
        str += "ERROR";
    }

    // View Camera
    if( getCamera() )
    {
        tmp = getCamera()->getName();
        tmp.replace( tmp.begin(), tmp.end(), ' ', '#');
        str += " ";
        str += tmp;
    } else
    {
        str += " NONE";
    }

    // Cull Camera
    if( getCull_camera() )
    {
        tmp = getCull_camera()->getName();
        tmp.replace( tmp.begin(), tmp.end(), ' ', '#');
        str += " ";
        str += tmp;
    } else
    {
        str += " NONE";
    }

    // Background Colour
    lsVector4 vec = getBackground_colour();
    str += " ";
    str += vec.toString();

    setSave_string( str );

    return getSave_string();
}

