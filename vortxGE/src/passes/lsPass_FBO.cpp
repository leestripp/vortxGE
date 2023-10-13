/*
 * Copyright (c) 2016 by Lee A. Stripp <leestripp@gmail.com>
 *
 * This file is part of the vortxGE library.
 *
 * vortxGE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * vortxGE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with vortxGE.  If not, see <http://www.gnu.org/licenses/>
 */

// OpenGL
#include "../opengl/lsOpenGL.h"

// datablocks
#include "../datablocks/lsStats.h"
#include "../datablocks/lsFBO.h"

// Scene
#include "../scene/lsScene.h"
#include "../scene/lsCamera.h"

#include "lsPass_FBO.h"

lsPass_FBO::lsPass_FBO( lsOpenGL* opengl ) : lsPass(opengl)
{
    setPass_type( LSPT_FBO );
    setName("Pass : FBO");

    fbo = NULL;

}

lsPass_FBO::~lsPass_FBO()
{
    // fbo's are not assets, so must cleanup
    if( fbo ) delete fbo;
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
    if( getScene()  ) getScene()->Optimize( getCull_camera(), this, NULL, getOgl() );

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

    if( fbo )
    {
        // sRGB
        glEnable( GL_FRAMEBUFFER_SRGB );

		// set viewport
		glViewport(getPosition().x, getPosition().y, getSize().x, getSize().y );

        getOgl()->StoreFBO();
        fbo->Enable();
        glClearColor( getBackground_colour().x, getBackground_colour().y, getBackground_colour().z, getBackground_colour().w );
        glClearDepth( 1.0 );
        glEnable( GL_DEPTH_TEST );
        glDepthMask( GL_TRUE );
        fbo->ClearBuffers();

        // Draw
        if( getScene() ) getScene()->Draw( getCamera(), this, NULL, getOgl() );

        getOgl()->RestoreFBO();
    }// if fbo
}

void lsPass_FBO::Resize( int x, int y, int width, int height  )
{
	// Resize view if needed
	setPosition( lsVector2(x,y) );
	setSize( lsVector2( width, height ) );
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
}

void lsPass_FBO::setFBO( lsFBO* val )
{
    fbo = val;
    setSize( lsVector2( fbo->getWidth(), fbo->getHeight() ) );
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

