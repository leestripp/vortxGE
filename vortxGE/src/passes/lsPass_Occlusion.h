/**
 * @file lsPass_Occlusion.h
 * @brief Occlusion query pass for culling invisible objects
 * 
 * lsPass_Occlusion performs occlusion culling to optimize rendering
 * by skipping objects that are hidden behind other geometry (e.g., walls).
 * This is useful for scenes with significant occluders like buildings.
 * 
 * @section occlusion_culling Occlusion Culling
 * 
 * The occlusion pass works by:
 * 1. Rendering coarse bounding geometry first
 * 2. Using occlusion queries to test visibility
 * 3. Only rendering fully visible objects in subsequent passes
 * 
 * @warning Occlusion culling adds GPU overhead. Most beneficial for
 * scenes with expensive-to-render objects and significant occlusion.
 * 
 * @see lsPass for base class
 * @see lsFBO_ColourDepth_8 for occlusion framebuffer
 */

#ifndef LSPASS_OCCLUSION_H
#define LSPASS_OCCLUSION_H

#include "lsPass.h"

// Forward declarations
class lsPassManager;
class lsNode;
class lsRender;
class lsFBO_ColourDepth_8;

/**
 * @class lsPass_Occlusion
 * @brief Occlusion query rendering pass
 * 
 * This pass performs hardware occlusion queries to determine
 * which objects are visible through potential occluders.
 * Objects that are fully occluded can be skipped in later passes.
 * 
 * @section usage Usage
 * 
 * @code
 * lsPass_Occlusion* occPass = new lsPass_Occlusion(0, 0, width, height, renderer);
 * occPass->setScene(scene);
 * occPass->setCamera(camera);
 * // Run before main render pass
 * @endcode
 * 
 * @note Best used with complex scenes with significant occlusion
 * @note May not provide benefit for simple scenes or those without occluders
 */

class lsPass_Occlusion : public lsPass
{
public:
    /**
     * @brief Construct an occlusion pass
     * 
     * @param x Viewport X position
     * @param y Viewport Y position
     * @param width Viewport width
     * @param height Viewport height
     * @param render Pointer to the renderer
     */
	lsPass_Occlusion( int x, int y, int width, int height, lsRender *render );
	
    /**
     * @brief Destructor
     */
	virtual ~lsPass_Occlusion();

    //=========================================================================
    // Virtual Methods
    //=========================================================================
    
    /**
     * @brief Update pass state
     * 
     * Updates occlusion query state and prepares for testing.
     */
    void Update();
    
    /**
     * @brief Execute occlusion queries
     * 
     * Renders bounding geometry and performs occlusion
     * queries to determine object visibility.
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
    /// Framebuffer for occlusion rendering
	lsFBO_ColourDepth_8 *fbo;

};

#endif // LSPASS_OCCLUSION_H
