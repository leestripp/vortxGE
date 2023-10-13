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

#include "lsBatch.h"

lsBatch::lsBatch( lsOpenGL *opengl )
{
    ogl = opengl;
	
    next_batch = NULL;
    prev_batch = NULL;

    batch_type = LSBT_NONE;
    name = "Not set";
    shader = NULL;
    vertex_count = 0;
	
}

lsBatch::~lsBatch()
{
#if defined(VORTX_DEBUG_BATCH)
    cout << "lsBatch::~lsBatch : Delete " << name << endl;
#endif

    // Batches are not assets, so must be deleted
    if( next_batch ) delete next_batch;
}

lsOpenGL *lsBatch::getOgl() const
{
    return ogl;
}

void lsBatch::setOgl(lsOpenGL *value)
{
    ogl = value;
}

int lsBatch::getVertex_count() const
{
    return vertex_count;
}

void lsBatch::setVertex_count(int value)
{
    vertex_count = value;
}

