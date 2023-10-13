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

// datablocks
#include "../datablocks/lsData.h"
#include "../datablocks/lsMesh.h"
#include "../datablocks/lsStats.h"

#include "lsAsset_Mesh.h"

/*! Mesh Asset
 * Asset Manager sould creat this instance.
 * */
lsAsset_Mesh::lsAsset_Mesh( lsOpenGL *opengl ) : lsAsset(opengl)
{
    setAsset_type( LSAT_MESH );
    setAsset_name( "Mesh" );
}

lsAsset_Mesh::~lsAsset_Mesh()
{
    FreeAsset();
}

// #######################
// lsAsset

/*!
 * \brief lsAsset_Mesh::AllocAsset
 *
 * Make sure you set the filename before you call this.\n
 *
 */
void lsAsset_Mesh::AllocAsset()
{
#ifdef VORTX_DEBUG_ASSETS
    cout << "lsAsset_Mesh::AllocAsset()" << endl;
#endif

    lsMesh *mesh;

    // Clear old data
    if( getData() )
    {
        delete getData();
        setData( NULL );
    }
    if(! getOpenGL() ) return;

    mesh = new lsMesh( getOpenGL() );
    if( mesh )
    {
        mesh->LoadVBM( filename, getAssetManager(), getOpenGL()->getShaderManager() );
        setData( mesh );
		// memory used
		getOpenGL()->getStats()->addUsed_mem( mesh->getMem_used() );
		
    } else
    {
        // Error
        cout << "ERROR: Could not create lsMesh instance - lsAsset_Mesh::AllocAsset()" << endl;
    }
}

void lsAsset_Mesh::FreeAsset()
{
    if( getData() )
	{
		// memory used
		getOpenGL()->getStats()->addUsed_mem( -getData()->getMem_used() );
		
        delete getData();
        setData( NULL );
    }
}

