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

#ifndef LSFBO_H
#define LSFBO_H

// opengl
#include "../opengl/lsOpenGL.h"

const int LSFT_NONE             = 0;
const int LSFT_COLOURDEPTH_8	= 1;
const int LSFT_COLOURDEPTH_16F	= 2;
const int LSFT_COLOURDEPTH_32F	= 3;
const int LSFT_DEPTH            = 4;
const int LSFT_DEPTH_16F        = 5;


using namespace std;


class lsFBO
{
public:
    lsFBO( lsOpenGL *opengl );
    virtual ~lsFBO();

    // Subclass
    virtual void Enable() = 0;
    virtual void ClearBuffers() = 0;

    // get set
    int getFBO_type() const
    {
        return fbo_type;
    }

    void setFBO_type( int val )
    {
        fbo_type = val;
    }

    // dimentions

    void setWidth( float val )
    {
        width = val;
    }

    float getWidth() const
    {
        return width;
    }

    void setHeight( float val )
    {
        height = val;
    }

    float getHeight() const
    {
        return height;
    }

    lsOpenGL *getOgl() const;
    void setOgl(lsOpenGL *value);

private:
    int fbo_type;
    float width;
    float height;

    // store link
    lsOpenGL *ogl;

};

#endif // LSFBO_H
