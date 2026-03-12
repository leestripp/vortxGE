/**
 * @file lsFBO_ColourDepth_8.h
 * @brief Color and depth framebuffer (8-bit)
 * 
 * lsFBO_ColourDepth_8 creates a framebuffer with both color and depth
 * buffers at 8-bit precision. This is the most common FBO type used for:
 * - Post-processing effects
 * - Multi-pass rendering
 * - Screen-space operations
 * - General off-screen rendering
 * 
 * @section typical_use Typical Use
 * 
 * @code
 * lsFBO_ColourDepth_8* fbo = new lsFBO_ColourDepth_8(width, height, renderer);
 * fbo->Enable();
 * // Render scene...
 * fbo->ClearBuffers();
 * 
 * // Use rendered texture
 * GLuint colorTex = fbo->getTexture();
 * GLuint depthTex = fbo->getDepthTexture();
 * @endcode
 * 
 * @note Color texture can be used for post-processing
 * @note Depth texture available for depth-based effects
 * @note 8-bit precision suitable for standard display
 * 
 * @see lsFBO for base class
 * @see lsFBO_Depth for depth-only FBO
 * @see lsFBO_ColourDepth_16F for HDR rendering
 */

#ifndef LSFBO_COLOURDEPTH_8_H
#define LSFBO_COLOURDEPTH_8_H

#include "../render/lsRender.h"

#include "lsFBO.h"


/**
 * @class lsFBO_ColourDepth_8
 * @brief Color + depth framebuffer with 8-bit precision
 * 
 * lsFBO_ColourDepth_8 provides a standard framebuffer with
 * color and depth attachments. The 8-bit color is suitable
 * for standard display output and most rendering effects.
 * 
 * @note For HDR/high-precision rendering, use lsFBO_ColourDepth_16F or 32F
 * @note iOS note: Store and restore default framebuffer on iOS
 */

class lsFBO_ColourDepth_8 : public lsFBO
{
public:
    /**
     * @brief Construct a color + depth FBO
     * 
     * @param width Framebuffer width
     * @param height Framebuffer height
     * @param render Pointer to the renderer
     */
	lsFBO_ColourDepth_8( double width, double height, lsRender *render );
	
    /**
     * @brief Destructor
     */
	virtual ~lsFBO_ColourDepth_8();

    //=========================================================================
    // lsFBO Virtual Methods
    //=========================================================================
    
    /**
     * @brief Bind the FBO
     * 
     * Binds the framebuffer as render target.
     * 
     * @note On iOS: Save current framebuffer before binding
     * @note On iOS: Restore default framebuffer after rendering
     */
	void Enable();
    
    /**
     * @brief Clear color and depth buffers
     */
	void ClearBuffers();

    //=========================================================================
    // Texture Access
    //=========================================================================
    
    /**
     * @brief Get the color texture
     * 
     * Returns the OpenGL texture ID containing rendered color data.
     * This texture can be used for post-processing or as input
     * for subsequent render passes.
     * 
     * @return OpenGL texture ID
     */
	GLuint getTexture()
	{
		return texture;
	}

	/**
     * @brief Get the depth texture
     * 
     * Returns the OpenGL texture ID containing depth values.
     * Useful for depth-based effects and manual depth testing.
     * 
     * @return OpenGL depth texture ID
     */
	GLuint getDepthTexture()
	{
		return depth_texture;
	}


private:
    /// Color attachment texture
	GLuint texture;
	
    /// Depth attachment texture
	GLuint depth_texture;

};

/*! @} */

#endif // LSFBO_COLOURDEPTH_8_H
