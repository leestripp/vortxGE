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

#ifndef vortxGE_lsTexture_h
#define vortxGE_lsTexture_h

#include <iostream>
#include <string>

// OpenGL
#include "../opengl/lsOpenGL.h"

#include "lsData.h"

class lsAsset;

const int LSIT_TGA	= 0;
const int LSIT_RGBE	= 1;
const int LSIT_EXR	= 2;

using namespace std;

/**
 OpenGL texture class for vortxGE
 \n
 Supported formats - TGA
 \n
 Holds a openGL texture with its width and height.

 */
class lsTexture : public lsData
{
public:
    lsTexture( lsOpenGL *opengl );
    virtual ~lsTexture();

    bool load_image( string filename, bool mipmap, bool compressed );

    // texture
    GLuint getTexture() const
    {
        return texture;
    }

    void setTexture( GLuint val )
    {
        texture = val;
    }

    int getImage_type() const
    {
        return image_type;
    }

    void setImage_type( int val )
    {
        image_type = val;
    }

    int getWidth() const
    {
        return width;
    }

    void setWidth( int val )
    {
        width = val;
    }

    int getHeight() const
    {
        return height;
    }

    void setHeight( int val )
    {
        height = val;
    }

    lsOpenGL *getOgl() const;
    void setOgl(lsOpenGL *value);
	
	
private:
    GLuint MakeGlTexture( GLenum Format, void *pixels, unsigned int w, unsigned int h, bool mipmap, bool compressed );
    int Get_ImageType( const string& filename );

    GLuint texture;
    int image_type;
    int width;
    int height;
	
    // opengl
    lsOpenGL *ogl;
};


#endif
