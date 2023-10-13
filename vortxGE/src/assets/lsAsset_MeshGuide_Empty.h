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

#ifndef vortxGE_lsAsset_MeshGuide_Empty_h
#define vortxGE_lsAsset_MeshGuide_Empty_h

#include "lsAsset.h"

class lsShaderManager;


/**
 * @brief Creates a Empty
 * 
 * Draws 2 lines from a center point for each axis.
 * X = Red, Y = Green, Z = Blue.
 * 
 * */

class lsAsset_MeshGuide_Empty : public lsAsset
{
public:
    lsAsset_MeshGuide_Empty( lsOpenGL *opengl );
    virtual ~lsAsset_MeshGuide_Empty();
    
    // lsAsset
    void AllocAsset();
    void FreeAsset();
    
private:
    
};


#endif
