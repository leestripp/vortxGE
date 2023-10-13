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

#ifndef LSASSET_H_
#define LSASSET_H_

#include <iostream>
#include <string>

// vortxGE
#include "../vortx_debug.h"

// opengl
#include "../opengl/lsOpenGL.h"

// types
const int LSAT_NONE                 = 0;
const int LSAT_TEXTURE              = 1;
const int LSAT_MESH                 = 2;
const int LSAT_BULLETSHAPE_PLANE    = 3;
const int LSAT_BULLETSHAPE_BOX      = 4;
const int LSAT_BULLETSHAPE_SPHERE   = 5;
const int LSAT_BULLETSHAPE_PILL     = 6;
const int LSAT_QUAD                 = 7;
// vortxEDIT
const int LSAT_MESHGUIDE_EMPTY      = 1000;
const int LSAT_MESHGUIDE_GRID       = 1001;


class lsAssetManager;
class lsData;

using namespace std;


/**
 * @brief Base class for all Assets
 *
 * To create a new asset use the functions in lsAssetManager
 *
 * AllocAsset() and FreeAsset() are used to manage your data, this makes it possibe to create
 * a dynamic asset when needed etc...
 *
 */
class lsAsset
{
public:
    lsAsset( lsOpenGL *ogl );
    virtual ~lsAsset();

    // User count
    void AddUser();
    void RemoveUser();

    // data
    virtual void AllocAsset() = 0;
    virtual void FreeAsset() = 0;

    // Asset Navi
    lsAsset *getNext_asset() const
    {
        return next_asset;
    }

    void setNext_asset(lsAsset *next_asset)
    {
        this->next_asset = next_asset;
    }

    lsAsset *getPrev_asset() const
    {
        return prev_asset;
    }

    void setPrev_asset(lsAsset *prev_asset)
    {
        this->prev_asset = prev_asset;
    }

    // Data

    int getAsset_type() const
    {
        return asset_type;
    }

    void setAsset_type( int val )
    {
        asset_type = val;
    }

    const string& getAsset_name()
    {
        return asset_name;
    }

    void setAsset_name( const string& name )
    {
        asset_name = name;
    }

    unsigned int getUsers() const
    {
        return users;
    }

    lsData* getData() const
    {
        return data;
    }

    void setData( lsData* val )
    {
        data = val;
    }

    // Manager

    lsAssetManager* getAssetManager() const
    {
        return assetmanager;
    }

    void setAssetManager( lsAssetManager* val )
    {
        assetmanager = val;
    }

    // opengl
    lsOpenGL* getOpenGL() const
    {
        return opengl;
    }

    void setOpenGL( lsOpenGL* val )
    {
        opengl = val;
    }

private:
    lsAsset* next_asset;
    lsAsset* prev_asset;
    // Managers
    lsAssetManager *assetmanager;
    lsOpenGL *opengl;

    // Data
    int asset_type;
    string asset_name;
    unsigned int    users;
    lsData *data;

};

#endif

