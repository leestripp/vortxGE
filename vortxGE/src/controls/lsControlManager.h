/**
 * @file lsControlManager.h
 * @brief Manager for all control systems
 * 
 * lsControlManager manages the control hierarchy in the engine.
 * It provides:
 * - Control registration
 * - Update/render orchestration
 * - Resize handling
 * 
 * @section usage Usage
 * @code
 * lsControlManager* cm = new lsControlManager(render);
 * 
 * // Add controls in order
 * cm->Add_Control(viewer);
 * cm->Add_Control(editor);
 * 
 * // Main loop
 * cm->Update(delta);
 * cm->Render();
 * 
 * // On window resize
 * cm->Resize(0, 0, width, height);
 * @endcode
 * 
 * @note Controls are rendered in reverse order added (last on top)
 * @see lsControl for base control class
 * @see lsViewer for scene viewer
 * @see lsComposite for layered rendering
 */

#ifndef vortxGE_lsControlManager_h
#define vortxGE_lsControlManager_h

#include <iostream>
#include <string>

// Forward declarations
class lsControl;
class lsRender;

/**
 * @class lsControlManager
 * @brief Manages all controls in the application
 * 
 * Acts as the top-level container for all control systems.
 * Orchestrates Update() and Render() calls to all controls.
 * 
 * @section order Control Order
 * 
 * Controls are added to the top of the list:
 * - First added = rendered first (background)
 * - Last added = rendered last (foreground)
 * 
 * @note Parent class is vortxGE, which calls Update/Render
 */
class lsControlManager
{
public:
    /**
     * @brief Construct control manager
     * @param render Pointer to renderer
     */
    lsControlManager( lsRender* render );
    
    /**
     * @brief Destructor
     * 
     * Cleans up all controls.
     */
    virtual ~lsControlManager();

    /**
     * @brief Update all controls
     * 
     * Calls Update() on each control in order.
     * @param delta Time since last frame
     */
    void Update( double delta );
    
    /**
     * @brief Render all controls
     * 
     * Calls Render() on each control in reverse order.
     */
    void Render();
    
    /**
     * @brief Handle window resize
     * 
     * Calls Resize() on each control.
     * @param x Position X
     * @param y Position Y
     * @param width New width
     * @param height New height
     */
	void Resize( int x, int y, int width, int height );

    /**
     * @brief Add control to manager
     * 
     * Adds control to top of list.
     * @param control Pointer to control
     */
    void Add_Control( lsControl* control );
    
    /**
     * @brief Remove control from manager
     * @param control Pointer to control
     */
    void Del_Control( lsControl* control );

    //=========================================================================
	// Accessors
	//=========================================================================
	
    /**
     * @brief Get control list
     * @return Pointer to first control
     */
    lsControl* getControl_list() const
    {
        return control_list;
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
    //=========================================================================
	// Private Members
	//=========================================================================
	
    /** @brief Head of control list */
    lsControl *control_list;

    /** @brief Renderer reference */
    lsRender *m_render;

};

#endif
