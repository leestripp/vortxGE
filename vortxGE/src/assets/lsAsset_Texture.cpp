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

// datablocks
#include "../datablocks/lsData.h"
#include "../datablocks/lsTexture.h"
#include "../datablocks/lsStats.h"

#include "lsAsset_Texture.h"

lsAsset_Texture::lsAsset_Texture( lsOpenGL *opengl ) : lsAsset(opengl)
{
    setAsset_type( LSAT_TEXTURE );
}

lsAsset_Texture::~lsAsset_Texture()
{
    FreeAsset();
}

// #######################
// lsAsset

/*!
 * \brief lsAsset_Texture::AllocAsset
 *
 * Make sure you set the filename before calling this.\n\
 *
 */
void lsAsset_Texture::AllocAsset()
{
    lsTexture *texture;

    // clean up
    FreeAsset();

    texture = new lsTexture( getOpenGL() );
    if( texture )
    {
        // This needs an fix
        texture->load_image( filename, true, false );

		setData( texture );
		// memory used
		getOpenGL()->getStats()->addUsed_mem( texture->getMem_used() );
		
    }
}

void lsAsset_Texture::FreeAsset()
{
    if( getData() )
    {
		// memory used
		getOpenGL()->getStats()->addUsed_mem( -getData()->getMem_used() );
		
        delete getData();
        setData( NULL );
    }
}

