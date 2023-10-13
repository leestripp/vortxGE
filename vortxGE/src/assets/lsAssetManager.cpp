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

#include "../vortx_debug.h"
#include "../vortxGE.h"

// filesystem
#include "../filesystem/lsDir.h"
#include "../filesystem/lsFileIO.h"

// datablocks
#include "../datablocks/lsString.h"
#include "../datablocks/lsStats.h"

// Assets
#include "lsAssetManager.h"
#include "lsAsset.h"
#include "lsAsset_Mesh.h"
#include "lsAsset_Quad.h"
#include "lsAsset_Texture.h"
// bullet
#include "lsAsset_BulletShape_Plane.h"
#include "lsAsset_BulletShape_Box.h"
// guides
#include "lsAsset_MeshGuide_Empty.h"
#include "lsAsset_MeshGuide_Grid.h"


lsAssetManager::lsAssetManager( lsOpenGL *opengl )
{
    // store link to opengl and shader manager thats part of lsOpenGL
    ogl = opengl;

    asset_list = NULL;
    asset_count = 0;

}

lsAssetManager::~lsAssetManager()
{
    while( asset_list )
    {
        Del_Asset( asset_list );
    }
}

/*!
 * \brief lsAssetManager::Add_Asset
 *
 * Sets the Asset Manager to self and adds the Asset to the list.\n
 *
 * \param asset
 */
void lsAssetManager::Add_Asset( lsAsset *asset )
{
    if(! asset ) return;
    asset_count++;

    // Add to list
    if( asset_list ) asset_list->setPrev_asset( asset );
    asset->setNext_asset( asset_list );
    asset_list = asset;
}

void lsAssetManager::Del_Asset( lsAsset* asset )
{
    if(! asset ) return;

    if( asset->getPrev_asset() == 0 )
    {
        // first in list
        asset_list = asset->getNext_asset();
        if( asset_list ) asset_list->setPrev_asset(0);

    } else
    {
        // Close the gap
        asset->getPrev_asset()->setNext_asset( asset->getNext_asset() );
        if( asset->getNext_asset() )
        {
            asset->getNext_asset()->setPrev_asset( asset->getPrev_asset() );
        }
    }
    asset_count--;
    delete asset;
}

lsAsset* lsAssetManager::Find_Asset( const string& name )
{
    lsAsset *list;

    list = asset_list;
    while( list )
    {
        if( list->getAsset_name() == name ) break;
        list = list->getNext_asset();
    }
    return list;
}

// ###############################
// create, Load and free

lsAsset_Mesh* lsAssetManager::CreateAsset_Mesh( const string& filename )
{
    lsAsset_Mesh *asset;

    asset = new lsAsset_Mesh( ogl );
    if( asset )
    {
        asset->setAssetManager( this );

        asset->setAsset_name( filename );
        asset->setFilename( filename );

        Add_Asset( asset );
    }

    return asset;
}

lsAsset_Quad* lsAssetManager::CreateAsset_Quad()
{
    lsAsset_Quad *asset;

    asset = new lsAsset_Quad( ogl );
    if( asset )
    {
        asset->setAssetManager( this );
        asset->setAsset_name( "Quad Mesh" );
        Add_Asset( asset );
    }
    return asset;
}

lsAsset_Texture* lsAssetManager::CreateAsset_Texture( const string& filename )
{
    lsAsset_Texture *asset;

    asset = new lsAsset_Texture( ogl );
    if( asset )
    {
        asset->setAssetManager( this );

        asset->setAsset_name( filename );
        asset->setFilename( filename );

        Add_Asset( asset );
    }

    return asset;
}

// Bullet Shapes

lsAsset_BulletShape_Plane* lsAssetManager::CreateAsset_BulletShape_Plane( float vectorX, float vectorY, float vectorZ )
{
    lsAsset_BulletShape_Plane *asset;

    asset = new lsAsset_BulletShape_Plane( vectorX, vectorY, vectorZ );
    if( asset )
    {
        asset->setAssetManager( this );

        Add_Asset( asset );
    }

    return asset;
}

lsAsset_BulletShape_Box* lsAssetManager::CreateAsset_BulletShape_Box( float x, float y, float z )
{
    lsAsset_BulletShape_Box *asset;

    asset = new lsAsset_BulletShape_Box( x, y, z );
    if( asset )
    {
        asset->setAssetManager( this );

        Add_Asset( asset );
    }

    return asset;
}

// *****************
// Guides

lsAsset_MeshGuide_Empty* lsAssetManager::CreateAsset_MeshGuide_Empty()
{
    lsAsset_MeshGuide_Empty *asset;

    asset = new lsAsset_MeshGuide_Empty( ogl );
    if( asset )
    {
        asset->setAssetManager( this );

        Add_Asset( asset );
    }

    return asset;
}

lsAsset_MeshGuide_Grid* lsAssetManager::CreateAsset_MeshGuide_Grid()
{
    lsAsset_MeshGuide_Grid *asset;

    asset = new lsAsset_MeshGuide_Grid( ogl );
    if( asset )
    {
        asset->setAssetManager( this );

        Add_Asset( asset );
    }

    return asset;
}

