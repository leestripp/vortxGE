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

#ifndef LSMESH_FACEGROUP_H
#define LSMESH_FACEGROUP_H

class lsMesh_Face;
class lsMaterial;
class lsShader_Program;


class lsMesh_FaceGroup
{
public:
    lsMesh_FaceGroup();
    virtual ~lsMesh_FaceGroup();

    void Add_Face( lsMesh_Face* face );

    void setMaterial( lsMaterial* mat );
    class lsMaterial* getMaterial();

    // Get set
    void setNext_group( lsMesh_FaceGroup* g );
    lsMesh_FaceGroup* getNext_group();
    class lsMesh_Face* getFace_list();
    void setFace_list( class lsMesh_Face* face );
    bool getSmooth_shade() const;
    void setSmooth_shade(bool value);
    int getTriangle_count();
    int getQuad_count();

    // Save

private:
    lsMesh_FaceGroup* next_facegroup;

    lsMesh_Face* face_list;

    class lsMaterial* material;
    int  triangle_count;
    int  quad_count;
    bool smooth_shade;

};

#endif // LSMESH_FACEGROUP_H
