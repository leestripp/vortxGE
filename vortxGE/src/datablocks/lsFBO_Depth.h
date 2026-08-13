/**
 * @file lsFBO_Depth.h
 * @brief Depth-only framebuffer for shadow mapping
 * 
 * lsFBO_Depth creates a depth-only framebuffer optimized for shadow
 * map generation. It only stores depth information, making it memory
 * efficient for shadow mapping applications.
 * 
 * @section shadow_mapping Shadow Mapping
 * 
 * Shadow mapping uses depth rendering:
 * 1. Render scene from light's perspective
 * 2. Store depth values in texture
 * 3. Compare in main pass to detect shadows
 * 
 * @section usage Usage
 * 
 * @code
 * lsFBO_Depth* shadowMap = new lsFBO_Depth(2048, 2048, renderer);
 * shadowMap->Enable();
 * // Render from light's view...
 * lsTextureID depthTex = shadowMap->getDepthTexture();
 * // Use in shadow shader...
 * @endcode
 * 
 * @see lsFBO for base class
 * @see lsPass_ShadowMap for shadow map generation pass
 * @see lsFBO_ColourDepth_8 for color + depth FBO
 */

#ifndef LSFBO_DEPTH_H
#define LSFBO_DEPTH_H

#include "lsFBO.h"

/**
 * @class lsFBO_Depth
 * @brief Depth-only framebuffer for shadow mapping
 * 
 * lsFBO_Depth creates a framebuffer with only a depth texture,
 * optimized for shadow map generation. This is more efficient
 * than storing color data which is unnecessary for shadows.
 * 
 * @note Depth texture uses LS_DEPTH_COMPONENT format
 * @note Commonly used with PCF (Percentage Closer Filtering) for soft shadows
 */

class lsFBO_Depth : public lsFBO
{
public:
    /**
     * @brief Construct a depth FBO
     * 
     * @param width Texture width (power of 2 recommended)
     * @param height Texture height (power of 2 recommended)
     * @param render Pointer to the renderer
     */
    lsFBO_Depth( double width, double height, lsRender *render );
    
    /**
     * @brief Destructor
     */
    virtual ~lsFBO_Depth();

    //=========================================================================
    // lsFBO Virtual Methods
    //=========================================================================
    
    /**
     * @brief Bind the depth FBO
     * 
     * Binds the FBO for depth-only rendering.
     */
    void Enable();
    
    /**
     * @brief Clear depth buffer
     */
    void ClearBuffers();

    /**
     * @brief Resize the FBO to new dimensions.
     *
     * @param width  New width in pixels
     * @param height New height in pixels
     */
    void Resize( double width, double height );

private:
    void CreateGL();

public:

    //=========================================================================
    // Texture Access
    //=========================================================================
    
    /**
     * @brief Get the depth texture
     * 
     * Returns the OpenGL texture ID containing depth values.
     * Use this texture in shadow calculation shaders.
     * 
     * @return OpenGL texture ID
     */
    lsTextureID getDepthTexture()
    {
        return depth_texture;
    }

    /**
     * @brief Raw (non-comparison) alias of the depth texture, or 0
     *
     * L4.1: the PCSS blocker search reads raw depth values; the main
     * texture is a comparison sampler. Same storage, compare mode off.
     */
    lsTextureID getDepthTextureRaw() const { return depth_texture_raw; }

private:
    /// Depth texture ID
    lsTextureID depth_texture;
    lsTextureID depth_texture_raw = 0; ///< L4.1: non-comparison alias (PCSS blocker search)
};

#endif // LSFBO_DEPTH_H
