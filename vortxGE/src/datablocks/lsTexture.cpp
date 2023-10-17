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

// datablocks
#include "lsString.h"
#include "lsTGA.h"

#include "lsTexture.h"

lsTexture::lsTexture( lsOpenGL *opengl ) : lsData()
{
    ogl = opengl;
    texture = 0;
}

lsTexture::~lsTexture()
{
    glDeleteTextures( 1, &texture );
}


/**
 Load Bitmap image
 \n
 Loads an image into a openGL texture.
*/
bool lsTexture::load_image( string filename, bool mipmap, bool compressed )
{
    unsigned int w=0,h=0,bpp=0;
    unsigned char* pixels=0;

    image_type = Get_ImageType( filename );

    if( image_type == LSIT_TGA )
    {
        lsTGA image;

        if( image.LoadTgaImage( filename.c_str(), &pixels, &w, &h, &bpp) )
        {
            width = w;
            height = h;

			// Memory
			setMem_used( image.getMem_used() );
			
            // generate the correct texture type for the image
            switch(bpp)
            {
            case 1:
                texture = MakeGlTexture( GL_ALPHA, pixels, w, h, mipmap, compressed );
                break;
            case 2:
                break;
            case 3:
                texture = MakeGlTexture( GL_RGB, pixels, w, h, mipmap, compressed );
                break;
            case 4:
                texture = MakeGlTexture( GL_RGBA, pixels, w, h, mipmap, compressed );
                break;
            default:
                break;
            }
            // delete the pixel data
            free( pixels );

            return true;
        }
        return false;
    }
    return false;
}


GLuint lsTexture::MakeGlTexture( GLenum Format, void *pixels, unsigned int w, unsigned int h, bool mipmap, bool compressed )
{
    GLuint texObject;

    // generate a texture object
    glGenTextures( 1, &texObject );

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glBindTexture( GL_TEXTURE_2D, texObject );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    // filter
    if( mipmap )
    {
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    } else
    {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    }

    //	glTexImage2D copies the texture data from system memory onto the graphics
    //	card. If we wish to use DXT compression then we simply change the internal
    //	format parameter to glTexImage2D.
    //
    if( compressed )
    {
        switch( Format )
        {
        case GL_RGB:
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
            // Do nothing
#else
            glTexImage2D( GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels );
#endif
            break;

        case GL_RGBA:
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
            // Do nothing
#else
            glTexImage2D( GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels );
#endif
            break;

        case GL_ALPHA:
            glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels );
            break;

        default:
            break;
        }
    } else
    {
        // uncompressed textures
        switch( Format )
        {
        default:
            glTexImage2D( GL_TEXTURE_2D, 0, Format, w, h, 0, Format, GL_UNSIGNED_BYTE, pixels );
            break;
        }

    }

    if( mipmap )
    {
        glGenerateMipmap( GL_TEXTURE_2D );
    }
    return texObject;
}

int lsTexture::Get_ImageType( const string& filename )
{
    lsString convert;
    vector<string> list;
    int result = -1;

    convert.SplitString( filename, '.', list );
    if( list[1] == "tga" )
    {
        result = LSIT_TGA;
    } else if( list[1] == "TGA" )
    {
        result = LSIT_TGA;
    } else
    {
        cout << "ERROR: unknown image format : " << list[1] << endl;
    }
    return result;
}

lsOpenGL *lsTexture::getOgl() const
{
    return ogl;
}

void lsTexture::setOgl(lsOpenGL *value)
{
    ogl = value;
}

