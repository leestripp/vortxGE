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

#ifndef vortxGE_lsAsset_Mesh_h
#define vortxGE_lsAsset_Mesh_h

#include "lsAsset.h"

class lsShaderManager;

/**
 * @brief Creates a Mesh Asset
 *
 */
class lsAsset_Mesh : public lsAsset
{
public:
    lsAsset_Mesh( lsOpenGL *opengl );
    virtual ~lsAsset_Mesh();

    // lsAsset
    void AllocAsset();
    void FreeAsset();

    // Data needed to alloc asset
    const string& getFilename() const
    {
        return filename;
    }

    void setFilename( const string& val )
    {
        filename = val;
    }

private:
    // Data needed to alloc asset
    string filename;

};


#endif
