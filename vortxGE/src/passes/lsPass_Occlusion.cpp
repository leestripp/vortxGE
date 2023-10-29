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

#include <iostream>
// debug
#include "../vortx_debug.h"

// OpenGL
#include "../opengl/lsOpenGL.h"

// datablocks
#include "../datablocks/lsFBO_ColourDepth_8.h"
#include "../datablocks/lsStats.h"

// Scene
#include "../scene/lsScene.h"
#include "../scene/lsObject.h"
#include "../scene/lsCamera.h"

#include "lsPass_Occlusion.h"


lsPass_Occlusion::lsPass_Occlusion( int x, int y, int width, int height, lsOpenGL *opengl ) : lsPass(opengl)
{
	setPass_type( LSPT_OCCLUSION );
	setName( "Occlusion Pass" );

	// render to fbo
	fbo = new lsFBO_ColourDepth_8( width, height, opengl );
	if(! fbo  )
	{
		cout << "lsPass_Occlusion::lsPass_Occlusion: ERROR - could not create lsFBO instance" << endl;
	}
	// set ratio
	Resize( x, y, width, height );
	setSize( lsVector2( width, height ) );

}

lsPass_Occlusion::~lsPass_Occlusion()
{
    if( fbo ) delete fbo;
}

void lsPass_Occlusion::Update()
{
#ifdef VORTX_DEBUG_RENDER
    cout << "lsPass_Occlusion::Update" << endl;
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

void lsPass_Occlusion::Render()
{
#ifdef VORTX_DEBUG_RENDER
    cout << "lsPass_Occlusion::Render" << endl;
#endif

	lsObject* list;
	GLuint id;
	GLint r;
	GLuint c, oc;

    if( fbo )
    {
        getOgl()->StoreFBO();

		// set viewport
		glViewport(getPosition().x, getPosition().y, getSize().x, getSize().y );
		
        // Set IDs and count draw objects
        oc = 0;
        // set IDs
        list = getScene()->getDraw_list();
        while( list )
        {
            if( list->getOcclusion_type() == LSOT_NONE )
            {
                // Generate occlusion query names
                glGenQueries( 1, &id );
                list->setOcclusionID( id );
            } else
            {
                list->setOcclusionID(0);
            }
            list->setOccluded( false );
            oc++;
            list = list->getNext_drawobject();
        } // while list

        // Draw occluders first
        fbo->Enable();
        glClearColor( 0.0, 0.0, 0.0, 1.0 );
        glClearDepth( 1.0 );
        glEnable( GL_DEPTH_TEST );
        glDepthMask( GL_TRUE );
        fbo->ClearBuffers();
        // Draw occluders
        list = getScene()->getDraw_list();
        while( list )
        {
            if( list->getOcclusion_type() == LSOT_OCCLUDER )
            {
                list->DrawSingle( getCamera(), this, getScene(), getOgl() );
#if defined(VORTX_DEBUG_OCCLUSION)
                cout << "Draw Occluder : " << list->getName() << endl;
#endif
            }
            list = list->getNext_drawobject();
        }

        // Draw volumes
        glShadeModel( GL_FLAT );
        glDepthMask( GL_FALSE );
        glColorMask( 0, 0, 0, 0 );

        list = getScene()->getDraw_list();
        while( list )
        {
            if( list->getOcclusion_type() == LSOT_NONE )
            {
                glBeginQuery( GL_SAMPLES_PASSED, list->getOcclusionID() );
                list->DrawSingle( getCamera(), this, getScene(), getOgl() );
                glEndQuery( GL_SAMPLES_PASSED );
            }
            list = list->getNext_drawobject();
        }

        glShadeModel( GL_SMOOTH );
        glDepthMask( GL_TRUE );
        glColorMask( 1, 1, 1, 1 );

        // Get and set result
        c = 0;
        list = getScene()->getDraw_list();
        while( list )
        {
            if( list->getOcclusion_type() == LSOT_NONE )
            {
                id = list->getOcclusionID();
                glGetQueryObjectiv( id, GL_QUERY_RESULT, &r );
                if( r == 0 )
                {
                    list->setOccluded( true );
                    c++;
                }
                // debug
#if defined(VORTX_DEBUG_OCCLUSION)
                cout << "Object : " << list->getName() << " - glGetQueryObjectiv Result : " << r << endl;
#endif
                glDeleteQueries( 1, &id );
            }
            list = list->getNext_drawobject();
        }

        glDisable( GL_CULL_FACE );
        getOgl()->RestoreFBO();
    }// if fbo

#if defined(VORTX_DEBUG_OCCLUSION)
  cout << "Draw Objects : " << oc << " ### Occlusion count : " << c << endl;
#endif
}

void lsPass_Occlusion::Resize( int x, int y, int width, int height  )
{
	// Resize view if needed
	setPosition( lsVector2(x,y) );
	setSize( lsVector2( width, height ) );
	float rw, rh;
	rw = width-x;
	rh = height - y;
	setVratio( rw / rh );
	
	// update camera
	if( height == 0 ) height = 1.0;
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


// ####################
// Save

const string& lsPass_Occlusion::toString()
{
    string str;
    string tmp;

    str = "add_pass occlusion ";
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

