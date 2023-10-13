/*
 * Copyright (C) 2015 by Lee A. Stripp <leestripp@gmail.com>
 *
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contributor(s):
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#include "lsMesh_FaceGroup.h"
#include "lsMesh_Face.h"

#include "lsMaterial.h"


lsMesh_FaceGroup::lsMesh_FaceGroup()
{
    next_facegroup = NULL;

    face_list = NULL;
    material = NULL;

    triangle_count = 0;
    quad_count = 0;
    smooth_shade = false;

}

lsMesh_FaceGroup::~lsMesh_FaceGroup()
{
    // DO NOT delete material

    if ( face_list ) delete face_list;
    if ( next_facegroup ) delete next_facegroup;
}

void lsMesh_FaceGroup::Add_Face( lsMesh_Face* face )
{
    face->setNext_face( face_list );
    face_list = face;

    if( face->getFace_type() == LSFT_QUAD )
    {
        face->setIndex( quad_count+1 ); // index starts at 1
        quad_count++;
    } else
    {
        face->setIndex( triangle_count+1 ); // index starts at 1
        triangle_count++;
    }
}

void lsMesh_FaceGroup::setMaterial( lsMaterial* mat )
{
    material = mat;
}

lsMaterial* lsMesh_FaceGroup::getMaterial()
{
    return material;
}

void lsMesh_FaceGroup::setNext_group( lsMesh_FaceGroup* g )
{
    next_facegroup = g;
}

lsMesh_FaceGroup* lsMesh_FaceGroup::getNext_group()
{
    return next_facegroup;
}

lsMesh_Face* lsMesh_FaceGroup::getFace_list()
{
    return face_list;
}

void lsMesh_FaceGroup::setFace_list( lsMesh_Face* face )
{
    face_list = face;
}


int lsMesh_FaceGroup::getTriangle_count()
{
    return triangle_count;
}

int lsMesh_FaceGroup::getQuad_count()
{
    return quad_count;
}

bool lsMesh_FaceGroup::getSmooth_shade() const
{
    return smooth_shade;
}

void lsMesh_FaceGroup::setSmooth_shade(bool value)
{
    smooth_shade = value;
}


