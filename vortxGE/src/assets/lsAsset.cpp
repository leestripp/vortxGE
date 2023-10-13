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

// core
#include "../vortx_debug.h"
#include "../vortxGE.h"

// Shaders
#include "../shader/lsShaderManager.h"

// datablocks
#include "../datablocks/lsData.h"
#include "../datablocks/lsMesh.h"
#include "../datablocks/lsTexture.h"

#include "lsAssetManager.h"
#include "lsAsset.h"


lsAsset::lsAsset( lsOpenGL *ogl )
{
    // Managers MUST be set by Asset Manager
    assetmanager = NULL;

    // Make sure we have access to opengl
    opengl = ogl;

    // Ser our pointers
    next_asset = NULL;
    prev_asset = NULL;

    // Data
    asset_type = LSAT_NONE;
    asset_name = "NOT set";
    users = 0;
    data = NULL;

}

lsAsset::~lsAsset()
{
    // dont delete managers
    // e.g. assetmanager

    if( data ) delete data;
}

void lsAsset::AddUser()
{
    users++;
}

void lsAsset::RemoveUser()
{
    users--;
}

