/**
 * @file lsFBO.h
 * @brief Base class for Framebuffer Objects (FBO)
 * 
 * lsFBO provides the foundation for off-screen rendering in OpenGL.
 * FBOs allow rendering to textures instead of the screen, enabling:
 * - Post-processing effects
 * - Shadow mapping
 * - Multi-pass rendering
 * - Screen-space effects
 * 
 * @section fbo_types FBO Types
 * 
 * - LSFT_NONE: No FBO
 * - LSFT_COLOURDEPTH_8: 8-bit color + depth
 * - LSFT_COLOURDEPTH_16F: 16-bit float color + depth
 * - LSFT_COLOURDEPTH_32F: 32-bit float color + depth
 * - LSFT_DEPTH: Depth-only texture
 * - LSFT_DEPTH_16F: 16-bit float depth
 * 
 * @section usage Usage
 * 
 * @code
 * lsFBO* fbo = new lsFBO_ColourDepth_8(width, height, renderer);
 * fbo->Enable();  // Bind FBO
 * // Render scene...
 * fbo->ClearBuffers();  // Clean up
 * @endcode
 * 
 * @see lsFBO_Depth for shadow mapping
 * @see lsFBO_ColourDepth_8 for color + depth rendering
 * @see lsPass_FBO for FBO rendering pass
 */

#ifndef LSFBO_H
#define LSFBO_H

// Render
#include "../render/lsRender.h"

//=========================================================================
// FBO Type Constants
//=========================================================================

/** @brief No FBO type */
const int LSFT_NONE             = 0;
/** @brief 8-bit color + depth */
const int LSFT_COLOURDEPTH_8	= 1;
/** @brief 16-bit float color + depth */
const int LSFT_COLOURDEPTH_16F	= 2;
/** @brief 32-bit float color + depth */
const int LSFT_COLOURDEPTH_32F	= 3;
/** @brief Depth-only texture */
const int LSFT_DEPTH            = 4;
/** @brief 16-bit float depth */
const int LSFT_DEPTH_16F        = 5;


using namespace std;

/**
 * @class lsFBO
 * @brief Base class for Framebuffer Objects
 * 
 * lsFBO manages OpenGL framebuffer objects for off-screen rendering.
 * Subclasses implement different buffer configurations based on
 * rendering needs (shadow maps, post-processing, etc.).
 * 
 * @note Must call Enable() before rendering to FBO
 * @note Must restore default framebuffer after rendering
 */

class lsFBO
{
public:
    /**
     * @brief Construct an FBO
     * @param render Pointer to the renderer
     */
    lsFBO( lsRender *render );
    
    /**
     * @brief Destructor
     */
    virtual ~lsFBO();

    //=========================================================================
    // Virtual Methods
    //=========================================================================
    
    /**
     * @brief Bind the FBO for rendering
     * 
     * Makes this FBO the current render target.
     * Call glBindFramebuffer(LS_FRAMEBUFFER, m_fbo) equivalent.
     */
    virtual void Enable() = 0;
    
    /**
     * @brief Clear FBO buffers
     *
     * Clears color, depth, and stencil buffers.
     */
    virtual void ClearBuffers() = 0;

    /**
     * @brief Resize the FBO to new dimensions
     *
     * Recreates the underlying framebuffer, colour textures, and depth
     * textures at the new size. Subclasses override to delete the old
     * GL resources and allocate new ones at the given pixel dimensions.
     *
     * @param width  New width in **physical pixels** (not logical units)
     * @param height New height in **physical pixels** (not logical units)
     *
     * @note Desktop-zoom / HiDPI contract:
     *  Callers MUST pass the framebuffer's *physical* pixel size, not
     *  the logical window size. On Wayland with 125% desktop scaling,
     *  a 1920x1080 logical window has a 2400x1350 physical framebuffer.
     *  Passing 1920x1080 would recreate the FBO at the wrong physical
     *  size and the rendered scene would only cover ~80% of the screen.
     *  The GLFW3 demo (`demos/vortxGEdemo_glfw3/src/glfwApp.cpp`) is
     *  responsible for computing the correct physical-pixel size via
     *  glfwGetFramebufferSize() × glfwGetWindowContentScale(). See
     *  docs/glfw_desktop_scale_factor.md for the full rationale.
     */
    virtual void Resize( double width, double height ) = 0;

    //=========================================================================
    // Properties
    //=========================================================================
    
    /**
     * @brief Get FBO type
     * @return LSFT_* constant
     */
    int getFBO_type() const
    {
        return fbo_type;
    }

    /**
     * @brief Set FBO type
     * @param val LSFT_* constant
     */
    void setFBO_type( int val )
    {
        fbo_type = val;
    }

    //=========================================================================
    // Dimensions
    //=========================================================================
    
    /**
     * @brief Set FBO width
     * @param val Width in pixels
     */
    void setWidth( double val )
    {
        width = val;
    }

    /**
     * @brief Get FBO width
     * @return Width in pixels
     */
    double getWidth() const
    {
        return width;
    }

    /**
     * @brief Set FBO height
     * @param val Height in pixels
     */
    void setHeight( double val )
    {
        height = val;
    }

    /**
     * @brief Get FBO height
     * @return Height in pixels
     */
    double getHeight() const
    {
        return height;
    }

    //=========================================================================
    // Renderer Access
    //=========================================================================
    
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
    // Public Members
    //=========================================================================
    
    /** @brief Renderer framebuffer object ID (lsFBOID = uint32_t). */
	lsFBOID m_fbo;

private:
    /// FBO type
    int fbo_type;
    
    /// Framebuffer width
    double width;
    
    /// Framebuffer height
    double height;

    /// Renderer reference
    lsRender *m_render;
};

#endif // LSFBO_H
