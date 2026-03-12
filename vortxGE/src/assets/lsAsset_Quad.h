/**
 * @file lsAsset_Quad.h
 * @brief Quad asset for full-screen effects
 * 
 * Asset type for creating a quad (2 triangles) that spans the
 * screen. Used for post-processing effects, render-to-texture,
 * and UI elements.
 * 
 * @author vortxGE
 * @date 2024
 * @see lsAsset
 * @see lsBatch_Quad
 * @see lsFBO
 */

#ifndef LSASSET_QUAD_H
#define LSASSET_QUAD_H

#include "lsAsset.h"

class lsBatch_Quad;

/**
 * @class lsAsset_Quad
 * @brief Asset for rendering screen-space quads
 * 
 * Represents a 2D quad that can be used for:
 * - Post-processing effects (bloom, blur, etc.)
 * - Render-to-texture operations
 * - UI overlay rendering
 * - Screen-space debugging
 * 
 * The quad has texture coordinates for sampling from textures
 * (typically from FBOs).
 * 
 * @note Asset type: LSAT_QUAD
 * 
 * @example
 * @code
 * // Create quad asset
 * lsAsset* quad = am->Add_Asset(LSAT_QUAD);
 * lsAsset_Quad* quadAsset = dynamic_cast<lsAsset_Quad*>(quad);
 * 
 * // Set FBO texture to sample from
 * quadAsset->Set_texture(fbo->getTexture());
 * 
 * // Render quad in a pass
 * lsBatch_Quad* batch = quadAsset->getBatch_Quad();
 * batch->Render(cam, pass, scene, render);
 * @endcode
 */
class lsAsset_Quad : public lsAsset
{
public:
	/**
	 * @brief Construct quad asset
	 * @param render Pointer to renderer
	 */
    lsAsset_Quad( lsRender *render );
    
    /// Destructor
    virtual ~lsAsset_Quad();

    //=========================================================================
    // lsAsset Override
    //=========================================================================
    
    /**
     * @brief Allocate quad geometry
     * 
     * Creates vertex buffers for the quad.
     */
    void AllocAsset();
    
    /**
     * @brief Free quad geometry
     */
    void FreeAsset();
    
    /**
     * @brief Set texture to sample from
     * 
     * Typically set to an FBO's texture for post-processing.
     * 
     * @param val OpenGL texture ID
     */
    void Set_texture( GLuint val );

    //=========================================================================
    // Accessors
    //=========================================================================
    
    /**
     * @brief Get quad batch for rendering
     * @return Pointer to lsBatch_Quad
     */
    lsBatch_Quad* getBatch_Quad() const
    {
        return batch_quad;
    }

private:
    /// Batch for rendering the quad
    lsBatch_Quad *batch_quad;

};

#endif // LSASSET_QUAD_H
