/**
 * @file lsPass_FBO.h
 * @brief Framebuffer Object (FBO) rendering pass
 * 
 * lsPass_FBO renders the scene to a Framebuffer Object instead of
 * the default screen buffer. This enables:
 * - Post-processing effects
 * - Multi-pass rendering techniques
 * - Off-screen rendering for textures
 * - Advanced effects like bloom, HDR, etc.
 * 
 * @see lsPass for base class
 * @see lsFBO for framebuffer management
 * @see lsPass_DR for direct screen rendering
 */

#ifndef vortxGE_lsPass_FBO_h
#define vortxGE_lsPass_FBO_h

#include "lsPass.h"

// Forward declaration
class lsFBO;

/**
 * @class lsPass_FBO
 * @brief Rendering pass that outputs to a framebuffer object
 * 
 * This pass renders to an FBO, allowing the rendered image to be
 * used as a texture for subsequent passes or post-processing.
 * 
 * @section usage Usage
 * 
 * @code
 * lsPass_FBO* fboPass = new lsPass_FBO(renderer);
 * lsFBO* fbo = new lsFBO_ColourDepth_8(width, height, renderer);
 * fboPass->setFBO(fbo);
 * fboPass->setScene(scene);
 * fboPass->setCamera(camera);
 * @endcode
 * 
 * @note The FBO must be properly configured before use
 * @note Use getFBO() to access the rendered texture
 */

class lsPass_FBO : public lsPass
{
public:
    /**
     * @brief Construct an FBO render pass
     * @param render Pointer to the renderer
     */
    lsPass_FBO( lsRender* render );
    
    /**
     * @brief Destructor
     */
    virtual ~lsPass_FBO();

    //=========================================================================
    // Virtual Methods
    //=========================================================================
    
    /**
     * @brief Update pass state
     * 
     * Called each frame to prepare for rendering.
     * Updates the FBO if needed.
     */
    void Update();
    
    /**
     * @brief Execute the FBO render pass
     * 
     * Binds the FBO, clears buffers, and renders
     * the scene to the framebuffer object.
     */
    void Render();
    
    /**
     * @brief Handle viewport resize
     * 
     * Updates viewport and FBO dimensions.
     * 
     * @param x Viewport X position
     * @param y Viewport Y position
     * @param width New viewport width
     * @param height New viewport height
     */
	void Resize( int x, int y, int width, int height  );

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
     * @brief Get the framebuffer object
     * @return Pointer to lsFBO
     */
    lsFBO* getFBO() const
    {
        return m_lsfbo;
    }
    
    /**
     * @brief Set the framebuffer object
     * @param val Pointer to lsFBO
     */
    void setFBO( lsFBO* val );

private:
    /// Framebuffer object for off-screen rendering
    lsFBO *m_lsfbo;

};


#endif
