
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
    if( getRender() ) getRender()->DeleteTextures( 1, &texture );
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

            // For HDR, internal format will be LS_RGB32F or LS_RGBA32F
            lsEnum format = (comp == 3) ? LS_RGB : LS_RGBA;
            texture = MakeGlTexture( format, float_image_data, w, h, mipmap, compressed, LS_FLOAT );
            
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
                texture = MakeGlTexture( LS_ALPHA, pixels, w, h, mipmap, compressed );
                break;
            case 2:
                break;
            case 3:
                texture = MakeGlTexture( LS_RGB, pixels, w, h, mipmap, compressed );
                break;
            case 4:
                texture = MakeGlTexture( LS_RGBA, pixels, w, h, mipmap, compressed );
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
lsTextureID lsTexture::MakeGlTexture( lsEnum Format, void *pixels, unsigned int w, unsigned int h, bool mipmap, bool compressed, lsEnum dataType )
{
    lsTextureID texObject;

    // generate a texture object
    getRender()->GenTextures( 1, &texObject );

    getRender()->PixelStorei( LS_UNPACK_ALIGNMENT, 1 );
    getRender()->BindTexture( LS_TEXTURE_2D, texObject );
    getRender()->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_WRAP_S, LS_REPEAT );
    getRender()->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_WRAP_T, LS_REPEAT );

    // filter
    if( mipmap )
    {
        getRender()->TexParameterf( LS_TEXTURE_2D, LS_TEXTURE_MIN_FILTER, LS_LINEAR_MIPMAP_LINEAR ); // Use mipmap linear for smoother transitions
        getRender()->TexParameterf( LS_TEXTURE_2D, LS_TEXTURE_MAG_FILTER, LS_LINEAR );
    } else
    {
        getRender()->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_MAG_FILTER, LS_LINEAR );
        getRender()->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_MIN_FILTER, LS_LINEAR );
    }

    // Determine internal format
    lsEnum internalFormat;
    if (dataType == LS_FLOAT) {
        if (Format == LS_RGB) internalFormat = LS_RGB32F;
        else if (Format == LS_RGBA) internalFormat = LS_RGBA32F;
        else internalFormat = Format; // Fallback for unsupported float formats
    } else {
        if (Format == LS_RGB) internalFormat = LS_SRGB8; // Use sRGB for LDR textures
        else if (Format == LS_RGBA) internalFormat = LS_SRGB8_ALPHA8;
        else if (Format == LS_ALPHA) internalFormat = LS_R8; // Assuming alpha-only grayscale
        else internalFormat = Format; // Fallback
    }

    //	getRender()->TexImage2D copies the texture data from system memory onto the graphics
    //	card. If we wish to use DXT compression then we simply change the internal
    //	format parameter.
    //
    // Note: compressed parameter is generally for DXT compression, which is often for LDR.
    // For HDR or float textures, DXT is not typically used.
    if( compressed )
    {
        // Handle DXT compression (for LDR images) - This part remains largely the same
        switch( Format )
        {
        case LS_RGB:
            getRender()->TexImage2D( LS_TEXTURE_2D, 0, LS_SRGB, w, h, 0, LS_RGB, LS_UNSIGNED_BYTE, pixels );
            break;

        case LS_RGBA:
            getRender()->TexImage2D( LS_TEXTURE_2D,0,LS_SRGB_ALPHA,w,h,0,LS_RGBA,LS_UNSIGNED_BYTE,pixels );
            break;

        case LS_ALPHA:
            getRender()->TexImage2D( LS_TEXTURE_2D, 0, LS_ALPHA, w, h, 0, LS_ALPHA, LS_UNSIGNED_BYTE, pixels );
            break;

        default:
            break;
        }
    } else
    {
        // uncompressed textures
        getRender()->TexImage2D( LS_TEXTURE_2D, 0, internalFormat, w, h, 0, Format, dataType, pixels );
    }

    if( mipmap )
    {
        getRender()->GenerateMipmap( LS_TEXTURE_2D );
    }
    getRender()->BindTexture( LS_TEXTURE_2D, 0 ); // Unbind texture
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
