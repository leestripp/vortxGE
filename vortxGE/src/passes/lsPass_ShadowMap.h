/**
 * @file lsPass_ShadowMap.h
 * @brief Shadow map generation pass for shadow mapping
 * 
 * lsPass_ShadowMap generates shadow maps used for real-time shadow
 * rendering. It renders the scene from the light's perspective into
 * a depth texture, which is then used in subsequent rendering passes
 * to determine shadow visibility.
 * 
 * @section shadow_mapping Shadow Mapping
 * 
 * Shadow mapping is a depth-image based rendering technique:
 * 1. Render scene from light's POV to depth texture
 * 2. Compare depth values in main pass to detect shadows
 * 3. Apply shadow factor to shaded fragments
 * 
 * @see lsPass for base class
 * @see lsFBO_Depth for depth framebuffer
 * @see lsPass_DR for main rendering pass
 */

#pragma once

#include "lsPass.h"

// Forward declaration
class lsFBO_Depth;

/**
 * @class lsPass_ShadowMap
 * @brief Shadow map generation pass
 * 
 * This pass renders depth from the light's perspective into
 * a depth texture. The resulting shadow map is used by
 * other passes to compute shadows.
 * 
 * @section usage Usage
 * 
 * @code
 * lsPass_ShadowMap* shadowPass = new lsPass_ShadowMap(0, 0, 2048, 2048, renderer);
 * shadowPass->setScene(scene);
 * shadowPass->setLight(light);
 * // Get the shadow map texture for use in shader
 * GLuint shadowMap = shadowPass->getFBO()->getDepthTexture();
 * @endcode
 * 
 * @note Shadow map resolution affects shadow quality (higher = better)
 * @note Consider using PCF for softer shadow edges
 */

class lsPass_ShadowMap : public lsPass
{
public:
    /**
     * @brief Construct a shadow map pass
     * 
     * @param x Viewport X position (typically 0)
     * @param y Viewport Y position (typically 0)
     * @param width Shadow map width (power of 2 recommended, e.g., 2048)
     * @param height Shadow map height (power of 2 recommended, e.g., 2048)
     * @param render Pointer to the renderer
     */
    lsPass_ShadowMap( int x, int y, int width, int height, lsRender *render );
    
    /**
     * @brief Destructor
     */
    virtual ~lsPass_ShadowMap();

    //=========================================================================
    // Virtual Methods
    //=========================================================================
    
    /**
     * @brief Update pass state
     * 
     * Updates the light's view matrix and other state
     * needed for shadow map generation.
     */
    void Update();
    
    /**
     * @brief Generate shadow map
     * 
     * Renders the scene from the light's perspective
     * into the depth framebuffer.
     */
    void Render();
    
    /**
     * @brief Handle viewport resize
     * 
     * @param x Viewport X position
     * @param y Viewport Y position
     * @param width New viewport width
     * @param height New viewport height
     */
    void Resize( int x, int y, int width, int height );

    //=========================================================================
    // Serialization
    //=========================================================================
    
    /**
     * @brief Serialize pass data
     * @return String representation of the pass
     */
    virtual const string& toString();

    //=========================================================================
    // FBO Access
    //=========================================================================
    
    /**
     * @brief Get the depth framebuffer
     * @return Pointer to lsFBO_Depth containing the shadow map
     */
    lsFBO_Depth* getFBO() const
    {
        return fbo;
    }

private:
    /// Depth framebuffer for shadow map storage
    lsFBO_Depth *fbo;
};
