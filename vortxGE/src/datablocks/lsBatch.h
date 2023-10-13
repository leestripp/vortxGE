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

#ifndef vortxGE_lsBatch_h
#define vortxGE_lsBatch_h

#include <iostream>
#include <cstdint>
#include <string>

// openGL
#include "../opengl/lsOpenGL.h"

// Math
#include "../math/lsBoundingSphere.h"

// Memory
#include "lsData.h"

class lsObject;
class lsCamera;
class lsPass;
class lsShader;

// types
const int LSBT_NONE         = 0;
const int LSBT_LINE         = 1;
const int LSBT_QUAD         = 2;
const int LSBT_NORMALS      = 3;
const int LSBT_BULLET       = 4;    // Triangle mesh for Bullet physics
const int LSBT_MESH         = 5;

using namespace std;

//! Base class for VBO data
/**
 Base class for VBO data

 */
class lsBatch : public lsData
{
public:
    lsBatch( lsOpenGL *opengl );
    virtual ~lsBatch();

    virtual void Draw( lsCamera *cam, lsObject *object, lsPass *pass ) = 0;
    virtual void Update( double delta ) = 0;

    // openGL data - on video card : needed by the asset system
    virtual void Copy_GLdata() = 0;
    virtual void Clear_GLdata() = 0;

    // Navi +++++

    lsBatch* getNext_batch() const
    {
        return next_batch;
    }

    void setNext_batch( lsBatch *val )
    {
        next_batch = val;
    }

    lsBatch* getPrev_batch() const
    {
        return prev_batch;
    }

    void setPrev_batch( lsBatch *val )
    {
        prev_batch = val;
    }

    // --------------------------------

    int getBatch_type() const
    {
        return batch_type;
    }

    void setBatch_type( int val )
    {
        batch_type = val;
    }

    const string& getName()
    {
        return name;
    }

    void setName( const string& val )
    {
        name = val;
    }

    lsShader* getShader() const
    {
        return shader;
    }

    void setShader( lsShader* val )
    {
        shader = val;
    }
	
    lsOpenGL *getOgl() const;
    void setOgl(lsOpenGL *value);
	
    int getVertex_count() const;
    void setVertex_count(int value);
	
private:
    lsBatch     *next_batch;
    lsBatch     *prev_batch;

    // opengl functions
    lsOpenGL *ogl;

    int         batch_type;
    string      name;
    lsShader    *shader;
    int         vertex_count;
	
public:
    // lsMesh hold overall bsphere
    lsBoundingSphere bSphere;
};

#endif
