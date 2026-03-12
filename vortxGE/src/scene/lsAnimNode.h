/**
 * @file lsAnimNode.h
 * @brief Animated node - base class for animatable scene objects
 * 
 * lsAnimNode extends lsNode with animation capabilities.
 * It provides:
 * - Animation manager integration
 * - Property system for customizable attributes
 * - Animation playback controls
 * 
 * @section usage Usage
 * @code
 * // Enable animation on an object
 * lsObject* obj = new lsObject(scene);
 * obj->Init_Animation(30.0);  // 30 FPS
 * 
 * // Control playback
 * obj->Play_Animation(true);   // Loop
 * obj->Stop_Animation();
 * @endcode
 * 
 * @note Remember to call Init_Animation() after creating
 *       the object to enable animation!
 * 
 * @see lsAnimationManager for animation system
 * @see lsObject for renderable objects
 */

#pragma once

// Forward declarations
class lsAnimationManager;
class lsProperty;

#include "lsNode.h"

/**
 * @class lsAnimNode
 * @brief Animated node - base for animatable objects
 * 
 * Extends lsNode with animation capabilities.
 * Used for objects that need keyframe animation
 * of transform or material properties.
 * 
 * @section properties Properties
 * 
 * Properties are customizable attributes that can
 * be animated:
 * - Transform (position, rotation, scale)
 * - Material (colors, shininess)
 * - Custom user-defined properties
 */
class lsAnimNode : public lsNode
{
public:
    /**
     * @brief Construct animated node
     * @param p Parent node
     */
	lsAnimNode( lsNode* p );
	
    /**
     * @brief Destructor
     */
	virtual ~lsAnimNode();
	
    //=========================================================================
	// Virtual Overrides
	//=========================================================================
	
    /**
     * @brief Update node with animation
     * @param scn Scene
     * @param render Renderer
     * @param delta Time since last frame
     */
	virtual void Move( lsScene* scn, lsRender* render, double delta );
	
    /**
     * @brief Optimize with animation
     * @param cam Camera
     * @param pass Render pass
     * @param scn Scene
     * @param render Renderer
     */
	virtual void Optimize( lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render );
	
    /**
     * @brief Draw with animation
     * @param cam Camera
     * @param pass Render pass
     * @param scn Scene
     * @param render Renderer
     */
	virtual void Draw( lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render );
	
    //=========================================================================
	// Scene Access
	//=========================================================================
	
    /**
     * @brief Get scene
     * @return Pointer to scene
     */
	virtual lsScene *Get_Scene();
	
    /**
     * @brief Get event manager
     * @return Pointer to event manager
     */
	virtual lsEventManager *Get_EventManager();
	
    //=========================================================================
	// Animation
	//=========================================================================
	
    /**
     * @brief Initialize animation system
     * 
     * Must be called after construction to enable animation!
     * 
     * @param fps Frames per second
     */
	void Init_Animation( double fps );
	
    /**
     * @brief Start animation playback
     * 
     * @param loop true to loop animation
     */
	void Play_Animation( bool loop );
	
    /**
     * @brief Stop animation
     */
	void Stop_Animation();
	
    //=========================================================================
	// Properties
	//=========================================================================
	
    /**
     * @brief Add property
     * @param p Property to add
     */
	void Add_Property( lsProperty *p );
	
    //=========================================================================
	// Serialization
	//=========================================================================
	
    /**
     * @brief Serialize node
     * @return String representation
     */
	virtual const string& toString();
	
    //=========================================================================
	// Accessors
	//=========================================================================
	
    /**
     * @brief Get animation manager
     * @return Pointer to animation manager
     */
	lsAnimationManager* getAnimationManager() const
	{
		return animationmanager;
	}
	
    /**
     * @brief Get property list
     * @return Pointer to first property
     */
	lsProperty* getProperty_list() const
	{
		return property_list;
	}
	
private:
    //=========================================================================
	// Private Members
	//=========================================================================
	
    /** @brief Animation manager */
	lsAnimationManager  *animationmanager;
	
    //=========================================================================
	// Properties
	//=========================================================================
    
    /** @brief Property list */
	lsProperty          *property_list;
};
