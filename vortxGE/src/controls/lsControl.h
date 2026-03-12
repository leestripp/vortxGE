/**
 * @file lsControl.h
 * @brief Base class for all control systems
 * 
 * lsControl is the base class for the control system hierarchy.
 * Controls manage:
 * - Scene rendering (Viewer, Composite)
 * - User input (menus, gameplay)
 * - Level loading
 * 
 * @section control_types Control Types
 * 
 * - LSCT_NONE: No type
 * - LSCT_COMPOSITE: Layered rendering control
 * - LSCT_MENU: Menu/input control
 * - LSCT_VIEWER: Simple scene viewer
 * - LSCT_LEVEL: Level control
 * - LSCT_LEVELLOADER: Level loading
 * - LSCT_GAMEPLAY: Gameplay control
 * - LSCT_GAME: Main game control
 * - LSCT_SANDBOX: Sandbox mode
 * - LSCT_SANDBOXZONE: Sandbox zone
 * 
 * @section usage Usage
 * @code
 * // Create control
 * lsViewer* viewer = new lsViewer(render, nullptr);
 * viewer->Init();
 * 
 * // In main loop:
 * viewer->Update(delta);
 * viewer->Render();
 * @endcode
 * 
 * @see lsControlManager for managing controls
 * @see lsViewer for simple scene viewer
 * @see lsComposite for layered rendering
 */

#ifndef vortxGE_lsControl_h
#define vortxGE_lsControl_h

#include <iostream>
#include <string>

using namespace std;

// Forward declarations
class vortxGE;
class lsRender;
class lsControlManager;
class lsShaderManager;
class lsAssetManager;

//=========================================================================
// Control Type Constants
//=========================================================================

/** @brief No type / undefined */
const int LSCT_NONE         = 0;
/** @brief Layered rendering composite */
const int LSCT_COMPOSITE    = 1;
/** @brief Menu/user interface control */
const int LSCT_MENU         = 2;
/** @brief Simple scene viewer */
const int LSCT_VIEWER       = 3;
/** @brief Level management control */
const int LSCT_LEVEL        = 4;
/** @brief Level loader control */
const int LSCT_LEVELLOADER  = 5;
/** @brief Gameplay control */
const int LSCT_GAMEPLAY     = 6;
/** @brief Main game control */
const int LSCT_GAME         = 7;
/** @brief Sandbox mode control */
const int LSCT_SANDBOX      = 8;
/** @brief Sandbox zone control */
const int LSCT_SANDBOXZONE  = 9;

/**
 * @class lsControl
 * @brief Base class for all control systems
 * 
 * Controls manage different aspects of the application:
 * - Rendering (via SceneManager)
 * - User input
 * - Game logic
 * 
 * Controls form a hierarchy via parent/child relationships.
 * 
 * @section lifecycle Lifecycle
 * 
 * Typical flow:
 * 1. Create control
 * 2. Call Init()
 * 3. Each frame: Update() then Render()
 * 4. On resize: Resize()
 */
class lsControl
{
public:
    /**
     * @brief Construct a control
     * @param render Pointer to renderer
     * @param p Parent control (can be nullptr)
     */
    lsControl( lsRender *render, lsControl* p );
    
    /**
     * @brief Destructor
     */
    virtual ~lsControl();

    //=========================================================================
	// Virtual Methods
	//=========================================================================
	
    /**
     * @brief Initialize the control
     * 
     * Called after construction to setup resources.
     * @return 0 on success
     */
    virtual int Init() = 0;

    /**
     * @brief Update control state
     * 
     * Called each frame to update logic.
     * @param delta Time since last frame
     */
    virtual void Update( double delta );
    
    /**
     * @brief Render the control
     * 
     * Called each frame to draw output.
     */
    virtual void Render();
    
    /**
     * @brief Handle resize
     * 
     * Called when window size changes.
     * @param x Position X
     * @param y Position Y
     * @param width New width
     * @param height New height
     */
	virtual void Resize( int x, int y, int width, int height );

    //=========================================================================
	// Hierarchy
	//=========================================================================
	
    /**
     * @brief Add child control
     * @param val Pointer to child control
     */
    void Add_Control( lsControl* val );

    //=========================================================================
	// Serialization
	//=========================================================================
	
    /**
     * @brief Get save string
     * @return Serialized data
     */
    const string& getSave_string()
    {
        return save_string;
    }

    /**
     * @brief Set save string
     * @param val Serialized data
     */
    void setSave_string( const string& val )
    {
        save_string = val;
    }

    //=========================================================================
	// Navigation
	//=========================================================================
	
    /**
     * @brief Get next control in list
     * @return Pointer to next control
     */
    lsControl* getNext_control() const
    {
        return next_control;
    }

    /**
     * @brief Set next control
     * @param val Pointer to next
     */
    void setNext_control( lsControl* val )
    {
        next_control = val;
    }

    /**
     * @brief Get previous control in list
     * @return Pointer to previous control
     */
    lsControl* getPrev_control() const
    {
        return prev_control;
    }

    /**
     * @brief Set previous control
     * @param val Pointer to previous
     */
    void setPrev_control( lsControl* val )
    {
        prev_control = val;
    }

    //=========================================================================
	// Settings
	//=========================================================================
	
    /**
     * @brief Get control type
     * @return LSCT_* constant
     */
    int getControl_type() const
    {
        return control_type;
    }

    /**
     * @brief Set control type
     * @param val LSCT_* constant
     */
    void setControl_type( int val )
    {
        control_type = val;
    }

    /**
     * @brief Get control name
     * @return Name string
     */
    const string& getName()
    {
        return name;
    }

    /**
     * @brief Set control name
     * @param val Name string
     */
    void setName( const string& val )
    {
        name = val;
    }

    /**
     * @brief Get active state
     * @return true if active
     */
    bool getActive() const
    {
        return active;
    }

    /**
     * @brief Set active state
     * @param val true to activate
     */
    void setActive( bool val )
    {
        active = val;
    }

    //=========================================================================
	// Hierarchy Access
	//=========================================================================
	
    /**
     * @brief Get control manager
     * @return Pointer to lsControlManager
     */
    lsControlManager* getControlManager() const
    {
        return controlmanager;
    }

    /**
     * @brief Get parent control
     * @return Pointer to parent lsControl
     */
    lsControl* getParent() const
    {
        return parent;
    }

    //=========================================================================
	// Render Access
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

private:
    //=========================================================================
	// Private Members
	//=========================================================================
	
    /** @brief Next control in list */
    lsControl   *next_control;
    
    /** @brief Previous control in list */
    lsControl   *prev_control;
    
    /** @brief Control manager */
    lsControlManager *controlmanager;
    
    /** @brief Parent control */
    lsControl *parent;

    /** @brief Renderer reference */
    lsRender *m_render;

    /** @brief Control type (LSCT_*) */
    int         control_type;
    
    /** @brief Control name */
    string      name;
    
    /** @brief Active state */
    bool        active;

    //=========================================================================
	// Serialization
	//=========================================================================
    
    /** @brief Save string */
    string save_string;

};

#endif
