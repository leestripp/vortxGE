#include <iostream>
// debug
#include "../vortx_debug.h"

// OpenGL
#include "../render/lsRender.h"

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
                    if( getOgl()->getStats() ) getOgl()->getStats()->Optimize_Start();
                    getScene()->OptimizeThread( getCull_camera(), this, getScene(), getOgl() );
                    if( getOgl()->getStats() ) getOgl()->getStats()->Optimize_Stop();
                }
            }
        } else
        {
            // Optimize stats
            if( getOgl()->getStats() ) getOgl()->getStats()->Optimize_Start();
            getScene()->OptimizeThread( getCull_camera(), this, getScene(), getOgl() );
            if( getOgl()->getStats() ) getOgl()->getStats()->Optimize_Stop();
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
        getOgl()->StoreDefaultFramebuffer();
		
        // use FBO
        m_lsfbo->Enable();
		
		// set viewport
		glViewport( getPosition().x, getPosition().y, getSize().x, getSize().y );

        glClearColor( getBackground_colour().x, getBackground_colour().y, getBackground_colour().z, getBackground_colour().w );
        glClearDepth( 1.0 );
        glEnable( GL_DEPTH_TEST );
        glDepthMask( GL_TRUE );
		// default depth range.
		glDepthRangef(0.0f, 1.0f);
		
        m_lsfbo->ClearBuffers();
		
        // Draw
        if( getScene() ) getScene()->Draw( getCamera(), this, NULL, getOgl() );

        // Make sure we are on the default framebuffer
        getOgl()->RestoreDefaultFramebuffer();

    } else
    {
        cout << "ERROR: lsPass_FBO::Render - lsFBO object not set" << endl;
    }// if fbo
}

void lsPass_FBO::Resize( int x, int y, int width, int height  )
{
	// // TODO: FBO are a set size for now.
	// setPosition( lsVector2(x,y) );
	// setSize( lsVector2( width, height ) );
	float rw, rh;
	rw = width-x;
	rh = height - y;
	setVratio( rw / rh );

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

