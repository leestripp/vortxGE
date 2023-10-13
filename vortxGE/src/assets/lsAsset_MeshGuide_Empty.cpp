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

// vortxGE
#include "../vortxGE.h"

// shader
#include "../shader/lsShaderManager.h"

// datablocks
#include "../datablocks/lsMesh.h"
#include "../datablocks/lsBatch_Lines.h"

// assets
#include "lsAsset_MeshGuide_Empty.h"


lsAsset_MeshGuide_Empty::lsAsset_MeshGuide_Empty( lsOpenGL *opengl ) : lsAsset(opengl)
{
    setAsset_type( LSAT_MESHGUIDE_EMPTY );
}

lsAsset_MeshGuide_Empty::~lsAsset_MeshGuide_Empty()
{
    FreeAsset();
}


// #######################
// lsAsset

void lsAsset_MeshGuide_Empty::AllocAsset()
{
    lsShader *shader;
    lsMesh *mesh;

    // Create Lines batch
    lsBatch_Lines *batch_lines = new lsBatch_Lines( getOpenGL() );
    if( batch_lines )
    {
        // Easy build Empty.
        batch_lines->buildEmpty( 1.0 );

        // set shader, lsBatch_Lines needs the debug shader
        shader = getOpenGL()->getShaderManager()->Find_Shader( "debug" );
        if( shader )
        {
            batch_lines->setShader( shader );
        }
        // Create mesh to store Batch
        mesh = new lsMesh( getOpenGL() );
        if( mesh )
        {
            // Add Batch to Mesh
            mesh->Add_Batch( batch_lines );
            // Store Mesh in Asset
            setData( mesh );
        }
    }
}

void lsAsset_MeshGuide_Empty::FreeAsset()
{
    if( getData() )
    {
        delete getData();
        setData( NULL );
    }
}

