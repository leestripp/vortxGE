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
 * lsTextureID colorTex = fbo->getTexture();
 * lsTextureID depthTex = fbo->getDepthTexture();
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

    /**
     * @brief Resize the FBO to new dimensions.
     *
     * Tears down the existing colour texture, depth texture, and FBO,
     * then recreates them at the new size. Useful when the window
     * framebuffer pixel size changes (HiDPI scaling, resize).
     *
     * @param width  New width in pixels
     * @param height New height in pixels
     */
	void Resize( double width, double height );

private:
    /**
     * @brief Create the GL resources (textures + FBO + attachments).
     *
     * Helper used by the constructor and by Resize(). Reads width and
     * height from the FBO's stored dimensions.
     */
	void CreateGL();

public:

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
	lsTextureID getTexture()
	{
		return texture;
	}

	/**
     * @brief Get the depth texture
     *
     * Returns the renderer texture ID containing depth values.
     * Useful for depth-based effects and manual depth testing.
     *
     * @return Renderer depth texture ID (lsTextureID = uint32_t)
     */
	lsTextureID getDepthTexture()
	{
		return depth_texture;
	}


private:
    /// Color attachment texture
	lsTextureID texture;

    /// Depth attachment texture
	lsTextureID depth_texture;

};

/*! @} */

#endif // LSFBO_COLOURDEPTH_8_H
