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
 * GLuint depthTex = shadowMap->getDepthTexture();
 * // Use in shadow shader...
 * @endcode
 * 
 * @see lsFBO for base class
 * @see lsPass_ShadowMap for shadow map generation pass
 * @see lsFBO_ColourDepth_8 for color + depth FBO
 */

#ifndef LSFBO_DEPTH_H
#define LSFBO_DEPTH_H

#include "../render/lsRender.h"

#include "lsFBO.h"

/**
 * @class lsFBO_Depth
 * @brief Depth-only framebuffer for shadow mapping
 * 
 * lsFBO_Depth creates a framebuffer with only a depth texture,
 * optimized for shadow map generation. This is more efficient
 * than storing color data which is unnecessary for shadows.
 * 
 * @note Depth texture uses GL_DEPTH_COMPONENT format
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
    GLuint getDepthTexture()
    {
        return depth_texture;
    }

private:
    /// Depth texture ID
    GLuint depth_texture;
};

#endif // LSFBO_DEPTH_H
