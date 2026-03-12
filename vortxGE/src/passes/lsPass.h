/**
 * @file lsPass.h
 * @brief Rendering pass base class for the render pipeline
 * 
 * lsPass represents a single rendering pass in the pipeline.
 * Passes control:
 * - What gets rendered (scene, camera, lights)
 * - Where it gets rendered (viewport, FBO)
 * - How it gets rendered (properties, settings)
 * 
 * @section pass_types Pass Types
 * 
 * - LSPT_NONE: No operation
 * - LSPT_DIRECTRENDER: Direct to screen rendering
 * - LSPT_FBO: Render to framebuffer object
 * - LSPT_SHADOWMAP: Shadow map generation
 * - LSPT_OCCLUSION: Occlusion query rendering
 * 
 * @section pipeline Pipeline
 * 
 * Typical render pipeline with multiple passes:
 * @code
 * lsPassManager* pm = scene->getPassManager();
 * for (lsPass* pass = pm->getPassList(); pass != nullptr; pass = pass->getNext_pass()) {
 *     pass->Update();
 *     pass->Render();
 * }
 * @endcode
 * 
 * @see lsPassManager for managing passes
 * @see lsPass_DR for direct rendering
 * @see lsPass_FBO for FBO rendering
 * @see lsPass_ShadowMap for shadow mapping
 */

#ifndef vortxGE_lsPass_h
#define vortxGE_lsPass_h

#include <iostream>
#include <string>

// math
#include "../math/lsVector2.h"
#include "../math/lsVector3.h"
#include "../math/lsVector4.h"

// Forward declarations
class lsScene;
class lsCamera;
class lsLight;
class lsRender;
class lsProperty;

//=========================================================================
// Pass Type Constants
//=========================================================================

/** @brief No operation pass */
const int LSPT_NONE             = 0;
/** @brief Direct rendering to screen */
const int LSPT_DIRECTRENDER     = 1;
/** @brief Render to framebuffer object */
const int LSPT_FBO              = 2;
/** @brief Shadow map generation */
const int LSPT_SHADOWMAP        = 3;
/** @brief Occlusion query rendering */
const int LSPT_OCCLUSION        = 4;

/**
 * @class lsPass
 * @brief Base class for rendering passes
 * 
 * A pass represents one stage in the rendering pipeline.
 * Each pass has its own camera, scene, and render target.
 * 
 * @section viewport Viewport
 * 
 * Each pass can have its own viewport:
 * - position: Screen position (x, y)
 * - size: Viewport dimensions (width, height)
 * - vratio: Aspect ratio
 * 
 * @section properties Properties
 * 
 * Passes can have properties for custom behavior:
 * - Post-processing effects
 * - Render settings
 * - Custom uniforms
 */
class lsPass
{
public:
    /**
     * @brief Construct a pass
     * @param render Pointer to renderer
     */
    lsPass( lsRender* render );
    
    /**
     * @brief Destructor
     */
    virtual ~lsPass();

    //=========================================================================
	// Virtual Methods
	//=========================================================================
	
    /**
     * @brief Update pass state
     * 
     * Called each frame to update pass data.
     */
    virtual void Update() = 0;
    
    /**
     * @brief Execute the pass
     * 
     * Performs the actual rendering.
     */
    virtual void Render() = 0;
    
    /**
     * @brief Handle viewport resize
     * 
     * @param x Position X
     * @param y Position Y
     * @param width New width
     * @param height New height
     */
	virtual void Resize( int x, int y, int width, int height ) = 0;

    /**
     * @brief Add property to pass
     * @param p Property to add
     */
    void Add_Property( lsProperty* p );

    //=========================================================================
	// Serialization
	//=========================================================================
	
    /**
     * @brief Serialize pass data
     * @return String representation
     */
    virtual const string& toString() = 0;

    //=========================================================================
	// Pass Chain Navigation
	//=========================================================================
	
    /**
     * @brief Get next pass in chain
     * @return Pointer to next lsPass
     */
    lsPass* getNext_pass() const
    {
        return next_pass;
    }

    /**
     * @brief Set next pass
     * @param val Pointer to next pass
     */
    void setNext_pass( lsPass* val )
    {
        next_pass = val;
    }

    /**
     * @brief Get previous pass
     * @return Pointer to previous lsPass
     */
    lsPass* getPrev_pass() const
    {
        return prev_pass;
    }

    /**
     * @brief Set previous pass
     * @param val Pointer to previous pass
     */
    void setPrev_pass( lsPass* val )
    {
        prev_pass = val;
    }

    //=========================================================================
	// Scene/Camera/Light Access
	//=========================================================================
	
    /**
     * @brief Get scene
     * @return Pointer to lsScene
     */
    lsScene* getScene() const
    {
        return scene;
    }

    /**
     * @brief Set scene
     * @param val Pointer to scene
     */
    void setScene( lsScene* val )
    {
        scene = val;
    }

    /**
     * @brief Get pass name
     * @return Name string
     */
    const string& getName()
    {
        return name;
    }

    /**
     * @brief Set pass name
     * @param val Name string
     */
    void setName( const string& val )
    {
        name = val;
    }

    /**
     * @brief Get pass type
     * @return LSPT_* constant
     */
    int getPass_type() const
    {
        return pass_type;
    }

    /**
     * @brief Set pass type
     * @param val LSPT_* constant
     */
    void setPass_type( int val )
    {
        pass_type = val;
    }

    /**
     * @brief Get camera
     * @return Pointer to lsCamera
     */
    lsCamera* getCamera() const
    {
        return camera;
    }

    /**
     * @brief Set camera
     * @param val Pointer to camera
     */
    void setCamera( lsCamera* val )
    {
        camera = val;
    }

    /**
     * @brief Get culling camera
     * @return Pointer to lsCamera for culling
     */
    lsCamera* getCull_camera() const
    {
        return cull_camera;
    }

    /**
     * @brief Set culling camera
     * @param val Pointer to culling camera
     */
    void setCull_camera( lsCamera* val )
    {
        cull_camera = val;
    }

    /**
     * @brief Get light
     * @return Pointer to lsLight
     */
    lsLight* getLight() const
    {
        return light;
    }

    /**
     * @brief Set light
     * @param val Pointer to light
     */
    void setLight( lsLight* val )
    {
        light = val;
    }

    //=========================================================================
	// Viewport
	//=========================================================================
	
    /**
     * @brief Get viewport position
     * @return Position vector (x, y)
     */
    const lsVector2& getPosition()
	{
		return position;
	}

	/**
     * @brief Set viewport position
     * @param val Position vector
     */
	void setPosition( const lsVector2& val )
	{
		position = val;
	}

    /**
     * @brief Get viewport size
     * @return Size vector (width, height)
     */
    const lsVector2& getSize()
    {
        return size;
    }

    /**
     * @brief Set viewport size
     * @param val Size vector
     */
    void setSize( const lsVector2& val )
    {
        size = val;
    }

    /**
     * @brief Get background color
     * @return RGBA color vector
     */
    const lsVector4& getBackground_colour()
    {
        return background_colour;
    }

    /**
     * @brief Set background color
     * @param val RGBA color vector
     */
    void setBackground_colour( const lsVector4& val )
    {
        background_colour = val;
    }

    /**
     * @brief Get aspect ratio
     * @return Width/height ratio
     */
    float getVratio() const
    {
        return vratio;
    }

    /**
     * @brief Set aspect ratio
     * @param val Width/height ratio
     */
    void setVratio( float val )
    {
        vratio = val;
    }

    //=========================================================================
	// Properties
	//=========================================================================
	
    /**
     * @brief Get property list
     * @return Pointer to first property
     */
    lsProperty* getProperty_list() const
    {
        return property_list;
    }

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

private:
    //=========================================================================
	// Private Members
	//=========================================================================
	
    /** @brief Next pass in chain */
    lsPass  *next_pass;
    
    /** @brief Previous pass in chain */
    lsPass  *prev_pass;

    /** @brief Renderer reference */
    lsRender *m_render;

    /** @brief Pass name */
    string name;
    
    /** @brief Pass type (LSPT_*) */
    int pass_type;

    /** @brief Scene to render */
    lsScene *scene;
    
    /** @brief Rendering camera */
    lsCamera *camera;
    
    /** @brief Culling camera (for frustum culling) */
    lsCamera *cull_camera;
    
    /** @brief Light for this pass */
    lsLight *light;

	//=========================================================================
	// Viewport
	//=========================================================================
	
    /** @brief Viewport position */
	lsVector2	position;
    
    /** @brief Viewport size */
    lsVector2   size;
    
    /** @brief Aspect ratio */
    float       vratio;
    
    /** @brief Background clear color */
    lsVector4   background_colour;

    //=========================================================================
	// Properties
	//=========================================================================
    
    /** @brief Property list */
    lsProperty *property_list;

    //=========================================================================
	// Serialization
	//=========================================================================
    
    /** @brief Serialized data */
    string save_string;

};


#endif
