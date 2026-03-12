/**
 * @file lsTexture.h
 * @brief OpenGL texture class
 * 
 * Low-level OpenGL texture management for loading and storing
 * 2D image data as GPU textures.
 * 
 * @author vortxGE
 * @date 2024
 * @see lsAsset_Texture
 * @see lsTGA
 */

#pragma once

#include <iostream>
#include <string>

// Render
#include "../render/lsRender.h"
#include "../render/lsOpenGL.h"

#include "lsData.h"

class lsAsset;

/// Image type constants
const int LSIT_TGA	= 0;	///< TGA image format
const int LSIT_RGBE	= 1;	///< RGBE (radiance) format
const int LSIT_EXR	= 2;	///< OpenEXR format
const int LSIT_HDR	= 3; 	///< HDR format

using namespace std;

/**
 * @class lsTexture
 * @brief OpenGL texture container
 * 
 * Manages an OpenGL texture object, its properties, and image data.
 * Used by the asset system to load and store textures.
 * 
 * Supported formats:
 * - TGA (primary)
 * - HDR (via float data)
 * 
 * @note This is a low-level class - typically use lsAsset_Texture
 * @note Inherits from lsData for memory tracking
 */
class lsTexture : public lsData
{
public:
	/**
	 * @brief Construct texture
	 * @param render Pointer to renderer
	 */
    lsTexture( lsRender *render );
    
    /// Destructor
    virtual ~lsTexture();

    //=========================================================================
    // Loading
    //=========================================================================
    
    /**
     * @brief Load image from file
     * 
     * @param filename Path to image file
     * @param mipmap Generate mipmaps
     * @param compressed Use compression
     * @param is_hdr Load as HDR float data
     * @return true if loaded successfully
     */
    bool load_image( string filename, bool mipmap, bool compressed, bool is_hdr = false );

    //=========================================================================
    // Texture Properties
    //=========================================================================
    
    /**
     * @brief Get OpenGL texture ID
     * @return GLuint texture handle
     */
    GLuint getTexture() const
    {
        return texture;
    }

    /**
     * @brief Set texture ID
     * @param val GLuint handle
     */
    void setTexture( GLuint val )
    {
        texture = val;
    }

    /**
     * @brief Get image type
     * @return Image type constant (LSIT_*)
     */
    int getImage_type() const
    {
        return image_type;
    }

    /**
     * @brief Set image type
     * @param val Image type constant
     */
    void setImage_type( int val )
    {
        image_type = val;
    }

    /**
     * @brief Get texture width
     * @return Width in pixels
     */
    int getWidth() const
    {
        return width;
    }

    /**
     * @brief Set texture width
     * @param val Width in pixels
     */
    void setWidth( int val )
    {
        this->width = val;
    }

    /**
     * @brief Get texture height
     * @return Height in pixels
     */
    int getHeight() const
    {
        return height;
    }

    /**
     * @brief Set texture height
     * @param val Height in pixels
     */
    void setHeight( int val )
    {
        this->height = val;
    }

    /**
     * @brief Get renderer
     * @return Pointer to lsRender
     */
    lsRender *getRender() const;
    
    /**
     * @brief Set renderer
     * @param value Pointer to renderer
     */
    void setOgl(lsRender *value);

    //=========================================================================
    // HDR Data
    //=========================================================================
    
    /**
     * @brief Get HDR float image data
     * @return Pointer to float data (or nullptr if not HDR)
     */
    float* getFloatImageData() const { return float_image_data; }
    
private:
    //=========================================================================
    // Private Methods
    //=========================================================================
    
    /**
     * @brief Create OpenGL texture from data
     * 
     * @param Format OpenGL format
     * @param pixels Pixel data
     * @param w Width
     * @param h Height
     * @param mipmap Generate mipmaps
     * @param compressed Use compression
     * @param dataType Data type (GL_UNSIGNED_BYTE or GL_FLOAT)
     * @return OpenGL texture ID
     */
    GLuint MakeGlTexture( GLenum Format, void *pixels, unsigned int w, unsigned int h, bool mipmap, bool compressed, GLenum dataType = GL_UNSIGNED_BYTE );
    
    /**
     * @brief Detect image type from filename
     * @param filename Image file path
     * @return Image type constant
     */
    int Get_ImageType( const string& filename );

    //=========================================================================
    // Private Members
    //=========================================================================
    
    GLuint texture;	///< OpenGL texture handle
    int image_type;	///< Image format type
    int width;		///< Texture width
    int height;		///< Texture height

    float* float_image_data; 	///< HDR float data (if applicable)
    
    // render
    lsRender *m_render;	///< Renderer reference
};
