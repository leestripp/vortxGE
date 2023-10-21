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

#ifndef vortxGE_lsBatch_Quad_h
#define vortxGE_lsBatch_Quad_h

// OpenGL
#include "../opengl/lsOpenGL.h"

// math
#include "../math/lsVector2.h"
#include "../math/lsVector3.h"
#include "../math/lsVector4.h"

#include "lsBatch.h"


class lsTexture;

//! Draw a quad
/**
 VBO Quad with texture coords.
 \n
 Creates and draws a 2x2 unit quad object. Pivot at center.

 */
class lsBatch_Quad : public lsBatch
{
public:
    lsBatch_Quad( lsOpenGL *opengl );
    virtual ~lsBatch_Quad();

    void Draw( lsCamera *cam, lsObject *object, lsPass *pass );
    void Update( double delta );

    // openGL data - on video card
    void Copy_GLdata();
    void Clear_GLdata();

    GLuint getTex_diffuse() const
    {
        return tex_diffuse;
    }

    void setTex_diffuse( GLuint val )
    {
        tex_diffuse = val;
    }

private:
    GLuint vao_quad;
	// Data
	GLuint vbo_verts;
	GLuint vbo_uvs;
	GLuint vbo_indices;

    // Textures
    GLuint      tex_diffuse;

};


#endif
