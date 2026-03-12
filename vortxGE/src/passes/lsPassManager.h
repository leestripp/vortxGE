/**
 * @file lsPassManager.h
 * @brief Manages the rendering pipeline passes
 * 
 * The pass manager handles the lifecycle of rendering passes in the engine.
 * It maintains a linked list of passes and coordinates their execution
 * during the render loop.
 * 
 * @author vortxGE
 * @date 2024
 * @see lsPass
 * @see lsPass_DR
 * @see lsPass_FBO
 */

#ifndef vortxGE_lsPassManager_h
#define vortxGE_lsPassManager_h

// Forward declarations
class lsPass;
class lsRender;

using namespace std;

/**
 * @class lsPassManager
 * @brief Manages rendering passes in the pipeline
 * 
 * The pass manager is responsible for:
 * - Maintaining the list of rendering passes
 * - Coordinating pass execution order
 * - Handling viewport resize events
 * - Managing pass lifecycle (add/remove)
 * 
 * @note Passes are executed in the order they were added.
 * Use Add_Pass and Del_Pass to modify the pipeline.
 * 
 * @example
 * @code
 * lsPassManager* pm = new lsPassManager(renderer);
 * 
 * // Add passes to pipeline
 * pm->Add_Pass(shadowPass);
 * pm->Add_Pass(mainPass);
 * pm->Add_Pass(postProcessPass);
 * 
 * // Render all passes
 * pm->Render();
 * 
 * // Handle resize
 * pm->Resize(0, 0, width, height);
 * @endcode
 */
class lsPassManager
{
public:
	/**
	 * @brief Construct pass manager
	 * 
	 * @param render Pointer to the renderer
	 */
    lsPassManager( lsRender* render );
    
    /// Destructor - cleans up pass list
    virtual ~lsPassManager();

    /**
     * @brief Render all passes in sequence
     * 
     * Iterates through all passes and calls their Render() method.
     * Each pass is responsible for its own rendering logic.
     */
    void Render();
    
    /**
     * @brief Handle window resize
     * 
     * Notifies all passes of viewport changes.
     * 
     * @param x New viewport X position
     * @param y New viewport Y position
     * @param width New viewport width
     * @param height New viewport height
     */
	void Resize( int x, int y, int width, int height  );

    /**
     * @brief Add a pass to the pipeline
     * 
     * Appends a pass to the end of the pass list.
     * 
     * @param pass Pointer to pass to add
     */
    void Add_Pass( lsPass* pass );
    
    /**
     * @brief Remove a pass from the pipeline
     * 
     * Removes the specified pass from the list.
     * 
     * @param pass Pointer to pass to remove
     */
    void Del_Pass( lsPass* pass );

    /**
     * @brief Get first pass in list
     * 
     * @return Pointer to first pass, or nullptr if empty
     */
    lsPass* getPass_list() const
    {
        return pass_list;
    }

    /**
     * @brief Set pass list head
     * 
     * @param val Pointer to new first pass
     */
    void setPass_list( lsPass* val )
    {
        pass_list = val;
    }

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

private:
    /// First pass in the pipeline
    lsPass      *pass_list;

    /// Renderer reference
    lsRender *m_render;

};


#endif
