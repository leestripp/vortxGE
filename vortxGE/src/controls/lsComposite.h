/**
 * @file lsComposite.h
 * @brief Layered rendering control with FBO support
 * 
 * lsComposite is a control for complex layered rendering.
 * It uses framebuffer objects (FBOs) to:
 * - Render scenes to textures
 * - Apply post-processing effects
 * - Combine multiple render passes
 * 
 * @section usage Usage
 * @code
 * lsComposite* composite = new lsComposite(render, nullptr);
 * composite->Init();
 * 
 * lsPassManager* pm = composite->getPassManager();
 * pm->Add_Pass(directRenderPass);
 * pm->Add_Pass(blurPass);
 * pm->Add_Pass(compositePass);
 * 
 * // Render pipeline: scene -> blur -> composite -> screen
 * @endcode
 * 
 * @note For simple rendering, use lsViewer instead
 * @see lsControl for base class
 * @see lsPass for render passes
 * @see lsFBO for framebuffer objects
 */

#ifndef vortxGE_lsComposite_h
#define vortxGE_lsComposite_h

#include "lsControl.h"

// Forward declarations
class lsSceneManager;
class lsPassManager;
class lsAssetManager;
class lsRender;

/**
 * @class lsComposite
 * @brief Layered rendering control
 * 
 * Provides advanced rendering with multiple passes:
 * - Render scenes to FBO textures
 * - Apply effects in sequence
 * - Combine layers for final output
 * 
 * @section pipeline Typical Pipeline
 * 
 * 1. Render scene to FBO (lsPass_FBO)
 * 2. Apply post-processing (blur, color correct)
 * 3. Composite layers together
 * 4. Output to screen
 */
class lsComposite : public lsControl
{
public:
    /**
     * @brief Construct composite control
     * @param render Pointer to renderer
     * @param p Parent control
     */
    lsComposite( lsRender *render, lsControl* p );
    
    /**
     * @brief Destructor
     */
    virtual ~lsComposite();

    /**
     * @brief Initialize composite
     * 
     * Creates managers and sets up pipeline.
     * @return 0 on success
     */
    int Init();

    //=========================================================================
	// Virtual Overrides
	//=========================================================================
	
    /**
     * @brief Update composite
     * @param delta Time since last frame
     */
    void Update( double delta );
    
    /**
     * @brief Render composite
     */
    void Render();
    
    /**
     * @brief Handle resize
     * @param x Position X
     * @param y Position Y
     * @param width New width
     * @param height New height
     */
	void Resize( int x, int y, int width, int height );

    //=========================================================================
	// Manager Access
	//=========================================================================
	
    /**
     * @brief Get scene manager
     * @return Pointer to lsSceneManager
     */
    lsSceneManager* getSceneManager() const
    {
        return scenemanager;
    }

    /**
     * @brief Get pass manager
     * @return Pointer to lsPassManager
     */
    lsPassManager* getPassManager() const
    {
        return passmanager;
    }

    /**
     * @brief Get asset manager
     * @return Pointer to lsAssetManager
     */
    lsAssetManager* getAssetManager() const
    {
        return assetmanager;
    }


private:
    //=========================================================================
	// Private Members
	//=========================================================================
	
    /** @brief Scene manager */
    lsSceneManager      *scenemanager;
    
    /** @brief Pass manager */
    lsPassManager       *passmanager;
    
    /** @brief Asset manager */
    lsAssetManager      *assetmanager;

};


#endif
