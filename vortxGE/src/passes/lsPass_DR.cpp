#include "../vortx_debug.h"

// OpenGL
#include "../opengl/lsOpenGL.h"

// datablocks
#include "../datablocks/lsStats.h"

// Scene
#include "../scene/lsScene.h"
#include "../scene/lsCamera.h"

#include "lsPass_DR.h"

lsPass_DR::lsPass_DR( lsOpenGL* opengl ) : lsPass(opengl)
{
    setPass_type( LSPT_DIRECTRENDER );
    setName("Pass : Direct Render");

}

lsPass_DR::~lsPass_DR()
{
}


/**
 Make sure our scene is ready for drawing
 */
void lsPass_DR::Update()
{
#ifdef VORTX_DEBUG_RENDER
    cout << "lsPass_DR::Update" << endl;
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

void lsPass_DR::Render()
{
#ifdef VORTX_DEBUG_RENDER
    cout << "lsPass_DR::Render" << endl;
#endif

    // TODO: workaround - how to find system FBO
    getOgl()->RestoreFBO();

    // set viewport
    glViewport(getPosition().x, getPosition().y, getSize().x, getSize().y );

    // Clear viewport
    glClearColor( getBackground_colour().x, getBackground_colour().y, getBackground_colour().z, getBackground_colour().w );
    glClearDepth( 1.0 );
    glEnable( GL_DEPTH_TEST );
    glDepthMask( GL_TRUE );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

#ifdef VORTX_DEBUG_OPENGLERRORS
    getOgl()->CheckGLErrors( getName() );
#endif // VORTX_DEBUG_OPENGLERRORS

    // Draw
    if( getScene() ) getScene()->Draw( getCamera(), this, NULL, getOgl() );
}

void lsPass_DR::Resize( int x, int y, int width, int height  )
{
#ifdef VORTX_DEBUG_PASSES
    cout << "lsPass_DR::Resize" << endl;
#endif

    // Resize view if needed
    if( width == 0 || height == 0 )
    {
        width = 620;
        height = 480;
    }
	setPosition( lsVector2(x,y) );
	setSize( lsVector2( width, height ) );
	float rw, rh;
	rw = width-x;
	rh = height - y;
	setVratio( rw / rh );

#ifdef VORTX_DEBUG_PASSES
	cout << "Pass area : (";
	cout << x << ", " << y << ", ";
	cout << width << ", " << height << ") ratio: ";
	cout << getVratio() << endl;
#endif

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

    // set viewport
    glViewport(getPosition().x, getPosition().y, getSize().x, getSize().y );

#ifdef VORTX_DEBUG_RENDER
    cout << "lsPass_DR::Resize - ";
    cout << "viewport Size: " << getSize().x << " x " << getSize().y << " vRatio: " << getVratio() << endl;
#endif

}


// ####################
// Save

const string& lsPass_DR::toString()
{
    string str;
    string tmp;

    str = "add_pass DR ";
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


