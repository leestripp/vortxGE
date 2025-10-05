
#include "../vortx_debug.h"

// datablocks
#include "lsString.h"
#include "lsTGA.h"

#include "lsTexture.h"

// tinygltf includes stb_image, ensure it's implemented.
#include "tinygltf/stb_image.h" // Only include the header.

lsTexture::lsTexture( lsRender *render ) : lsData()
{
    m_render = render;
    texture = 0;
    float_image_data = NULL; // Initialize new member
}

lsTexture::~lsTexture()
{
    glDeleteTextures( 1, &texture );
    if (float_image_data) { // Free HDR data
        stbi_image_free(float_image_data);
    }
}


/**
 Load Bitmap image
 \n
 Loads an image into a openGL texture.
*/
bool lsTexture::load_image( string filename, bool mipmap, bool compressed, bool is_hdr )
{
    unsigned int w=0,h=0,bpp=0;
    unsigned char* pixels=0;

    // Clean up previous float data if any
    if (float_image_data) {
        stbi_image_free(float_image_data);
        float_image_data = NULL;
    }

    // Determine image type based on file extension if not explicitly told it's HDR
    if (!is_hdr) {
        image_type = Get_ImageType( filename );
        if (image_type == LSIT_HDR) is_hdr = true; // Override based on extension
    } else {
        image_type = LSIT_HDR;
    }

    if( is_hdr ) {
        int comp;
        float_image_data = stbi_loadf(filename.c_str(), (int*)&w, (int*)&h, &comp, 0);
        if (float_image_data) {
            width = w;
            height = h;
            bpp = comp * sizeof(float); // Use float size
            setMem_used(w * h * bpp);

            // For HDR, internal format will be GL_RGB32F or GL_RGBA32F
            GLenum format = (comp == 3) ? GL_RGB : GL_RGBA;
            texture = MakeGlTexture( format, float_image_data, w, h, mipmap, compressed, GL_FLOAT );
            
            return true;
        } else {
            cout << "ERROR: Could not open HDR file: " << filename << endl;
            return false;
        }
    }
    else if( image_type == LSIT_TGA )
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


// Modified MakeGlTexture to accept dataType
GLuint lsTexture::MakeGlTexture( GLenum Format, void *pixels, unsigned int w, unsigned int h, bool mipmap, bool compressed, GLenum dataType )
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
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ); // Use mipmap linear for smoother transitions
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    } else
    {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    }

    // Determine internal format
    GLenum internalFormat;
    if (dataType == GL_FLOAT) {
        if (Format == GL_RGB) internalFormat = GL_RGB32F;
        else if (Format == GL_RGBA) internalFormat = GL_RGBA32F;
        else internalFormat = Format; // Fallback for unsupported float formats
    } else {
        if (Format == GL_RGB) internalFormat = GL_SRGB8; // Use sRGB for LDR textures
        else if (Format == GL_RGBA) internalFormat = GL_SRGB8_ALPHA8;
        else if (Format == GL_ALPHA) internalFormat = GL_R8; // Assuming alpha-only grayscale
        else internalFormat = Format; // Fallback
    }

    //	glTexImage2D copies the texture data from system memory onto the graphics
    //	card. If we wish to use DXT compression then we simply change the internal
    //	format parameter to glTexImage2D.
    //
    // Note: compressed parameter is generally for DXT compression, which is often for LDR.
    // For HDR or float textures, DXT is not typically used.
    if( compressed )
    {
        // Handle DXT compression (for LDR images) - This part remains largely the same
        switch( Format )
        {
        case GL_RGB:
            glTexImage2D( GL_TEXTURE_2D, 0, GL_SRGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels );
            break;

        case GL_RGBA:
            glTexImage2D( GL_TEXTURE_2D,0,GL_SRGB_ALPHA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels );
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
        glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, w, h, 0, Format, dataType, pixels );
    }

    if( mipmap )
    {
        glGenerateMipmap( GL_TEXTURE_2D );
    }
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture
    return texObject;
}

int lsTexture::Get_ImageType( const string& filename )
{
    lsString convert;
    vector<string> list;
    int result = -1;

    convert.SplitString( filename, '.', list );
    if( list.size() < 2 ) { // Not enough parts for an extension
        return result;
    }

    const string& ext = list[1]; // Extension is the second part

    if( ext == "tga" || ext == "TGA" )
    {
        result = LSIT_TGA;
    } else if( ext == "hdr" || ext == "HDR" ) // Check for HDR extension
    {
        result = LSIT_HDR;
    }
    else
    {
        cout << "ERROR: unknown image format : " << ext << endl;
    }
    return result;
}

lsRender *lsTexture::getRender() const
{
    return m_render;
}

void lsTexture::setOgl(lsRender *value)
{
    m_render = value;
}
