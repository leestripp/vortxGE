/**
 * @file lsPass_DR.h
 * @brief Direct rendering pass to the default framebuffer
 * 
 * lsPass_DR (Direct Render) is a rendering pass that outputs directly
 * to the system's default OpenGL framebuffer. This is the simplest
 * rendering pass type, suitable for final output to the screen.
 * 
 * @note Requires a default GL framebuffer with Colour and Depth buffers
 * @note This is typically the final pass in a render pipeline
 * 
 * @see lsPass for base class
 * @see lsPass_FBO for framebuffer rendering
 * @see lsPass_ShadowMap for shadow mapping
 */

#ifndef vortxGE_lsPass_DR_h
#define vortxGE_lsPass_DR_h

#include "lsPass.h"


/**
 * @class lsPass_DR
 * @brief Direct render pass - outputs to default framebuffer
 * 
 * This pass renders the scene directly to the default OpenGL
 * framebuffer (typically the back buffer for display).
 * Use this as the final pass in multi-pass rendering pipelines.
 * 
 * @section usage Usage
 * 
 * @code
 * lsPass_DR* drPass = new lsPass_DR(renderer);
 * drPass->setScene(scene);
 * drPass->setCamera(camera);
 * drPass->setName("Main Render");
 * @endcode
 * 
 * @note The default framebuffer must have color and depth buffers configured
 */

class lsPass_DR : public lsPass
{
public:
    /**
     * @brief Construct a direct render pass
     * @param render Pointer to the renderer
     */
    lsPass_DR( lsRender* render );
    
    /**
     * @brief Destructor
     */
    virtual ~lsPass_DR();

    //=========================================================================
    // Virtual Methods
    //=========================================================================
    
    /**
     * @brief Update pass state
     * 
     * Called each frame to prepare for rendering.
     * Updates internal state and prepares render targets.
     */
    void Update();
    
    /**
     * @brief Execute the direct render pass
     * 
     * Renders the scene to the default framebuffer.
     * Binds the default framebuffer, clears buffers,
     * and draws all visible objects.
     */
    void Render();
    
    /**
     * @brief Handle viewport resize
     * 
     * Updates viewport dimensions when the window is resized.
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

private:
    // Private implementation details

};


#endif
