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

#ifndef LSASSET_QUAD_H
#define LSASSET_QUAD_H

#include "lsAsset.h"

class lsBatch_Quad;


/**
 * @brief Creates a Quad
 * 
 * Draws a Quad with texture coords for use with render passes.
 * 
 * */
class lsAsset_Quad : public lsAsset
{
public:
    lsAsset_Quad( lsOpenGL *opengl );
    virtual ~lsAsset_Quad();

    // lsAsset
    void AllocAsset();
    void FreeAsset();
    // Set to your lsFBO texture
    void Set_texture( GLuint val );

    // Needed for render pipeline.
    lsBatch_Quad* getBatch_Quad() const
    {
        return batch_quad;
    }

private:
    lsBatch_Quad *batch_quad;

};

#endif // LSASSET_QUAD_H
