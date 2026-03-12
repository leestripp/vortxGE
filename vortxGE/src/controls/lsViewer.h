/**
 * @file lsViewer.h
 * @brief Simple scene viewer control
 * 
 * lsViewer is a control for simple scene rendering.
 * It combines:
 * - Scene manager (scene graph)
 * - Pass manager (rendering pipeline)
 * - Asset manager (resource loading)
 * 
 * Use for simple applications or testing.
 * 
 * @section usage Usage
 * @code
 * lsViewer* viewer = new lsViewer(render, nullptr);
 * viewer->Init();
 * 
 * lsSceneManager* sm = viewer->getSceneManager();
 * lsScene* scene = sm->Find_Scene("main");
 * 
 * // Main loop
 * viewer->Update(delta);
 * viewer->Render();
 * @endcode
 * 
 * @note For complex rendering, use lsComposite instead
 * @see lsControl for base class
 * @see lsSceneManager for scene management
 * @see lsPassManager for render passes
 */

#ifndef vortxGE_IOS_lsViewer_h
#define vortxGE_IOS_lsViewer_h

#include "lsControl.h"

// Forward declarations
class vortxGE;
class lsSceneManager;
class lsPassManager;
class lsAssetManager;
class lsRender;

/**
 * @class lsViewer
 * @brief Simple scene viewer control
 * 
 * Provides a simple way to render a scene.
 * Contains all necessary managers for basic rendering:
 * - SceneManager: manages scenes
 * - PassManager: manages render passes
 * - AssetManager: manages resources
 * 
 * @note Good for simple apps. Use lsComposite for
 *       multi-pass rendering with FBOs.
 */
class lsViewer : public lsControl
{
public:
    /**
     * @brief Construct viewer
     * @param render Pointer to renderer
     * @param p Parent control
     */
    lsViewer( lsRender *render, lsControl* p );
    
    /**
     * @brief Destructor
     */
    virtual ~lsViewer();

    /**
     * @brief Initialize viewer
     * 
     * Creates and initializes managers.
     * @return 0 on success
     */
    int Init();

    //=========================================================================
	// Virtual Overrides
	//=========================================================================
	
    /**
     * @brief Update viewer
     * @param delta Time since last frame
     */
    void Update( double delta );
    
    /**
     * @brief Render scene
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
