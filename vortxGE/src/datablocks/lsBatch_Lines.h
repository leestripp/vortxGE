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

#ifndef vortxGE_lsBatch_Lines_h
#define vortxGE_lsBatch_Lines_h

// math
#include "../math/lsVector2.h"
#include "../math/lsVector3.h"
#include "../math/lsVector4.h"

#include "lsBatch.h"

class lsOpenGL;


class lsBatch_Lines : public lsBatch
{
public:
    lsBatch_Lines( lsOpenGL *opengl );
    virtual ~lsBatch_Lines();

    void Draw( lsCamera *cam, lsObject *object, lsPass *pass );
    void Update( double delta );

    // Add lines
    void Begin( GLuint nMaxVerts );
    void AddLine( Vector3f verts[2], Vector4f vColours[2] );
    void End();
    void Cleanup();

    // The easy way
    void buildEmpty( GLfloat scale );
    void buildGrid();

    // openGL data - on video card : needed by the asset system
    void Copy_GLdata();
    void Clear_GLdata();

private:
    GLuint nNumVerts;

    // Data
    GLuint vbo_verts;
    GLuint vbo_colours;

    Vector3f *pVerts;
    Vector4f *pColours;

};


#endif
