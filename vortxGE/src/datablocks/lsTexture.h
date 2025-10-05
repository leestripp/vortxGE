
#pragma once

#include <iostream>
#include <string>

// Render
#include "../render/lsRender.h"
#include "../render/lsOpenGL.h"

#include "lsData.h"

class lsAsset;

const int LSIT_TGA	= 0;
const int LSIT_RGBE	= 1;
const int LSIT_EXR	= 2;
const int LSIT_HDR	= 3; // Added for HDR image type

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
    lsTexture( lsRender *render );
    virtual ~lsTexture();

    // Modified to handle HDR
    bool load_image( string filename, bool mipmap, bool compressed, bool is_hdr = false );

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
        this->width = val;
    }

    int getHeight() const
    {
        return height;
    }

    void setHeight( int val )
    {
        this->height = val;
    }

    lsRender *getRender() const;
    void setOgl(lsRender *value);

    // Add a method to get raw float data for HDR processing if needed elsewhere
    float* getFloatImageData() const { return float_image_data; }
    
private:
    // Modified to handle float data for HDR
    GLuint MakeGlTexture( GLenum Format, void *pixels, unsigned int w, unsigned int h, bool mipmap, bool compressed, GLenum dataType = GL_UNSIGNED_BYTE );
    int Get_ImageType( const string& filename );

    GLuint texture;
    int image_type;
    int width;
    int height;

    float* float_image_data; // To store HDR float data
    
    // render
    lsRender *m_render;
};
