/**
 * @file lsAnimationManager.h
 * @brief Animation manager for keyframe-based property animation
 * 
 * lsAnimationManager manages keyframe animations for animatable nodes.
 * It provides:
 * - Animation playback (play, loop, stop)
 * - Keyframe insertion for transforms
 * - Material property animation (color, specular)
 * - Frame-based animation at specified FPS
 * 
 * @section usage Usage
 * @code
 * // Initialize animation system
 * lsAnimationManager* anim = new lsAnimationManager();
 * anim->Init(myAnimNode, 30.0);  // 30 FPS
 * 
 * // Insert keyframes
 * anim->InsertKey_Pos(0.0, lsVector3(0, 0, 0));
 * anim->InsertKey_Pos(30.0, lsVector3(10, 0, 0));
 * anim->InsertKey_Pos(60.0, lsVector3(0, 0, 0));
 * 
 * // Play animation
 * anim->Play(true);  // Loop
 * 
 * // In update loop:
 * anim->UpdateAll(deltaTime);
 * @endcode
 * 
 * @see lsAnimNode for animatable nodes
 * @see lsAnimProperty for property animation
 * @see lsKeyFrame for keyframe data
 */

#ifndef LSANIMATIONMANAGER_H
#define LSANIMATIONMANAGER_H

// timers
#include "../timers/lsTimer.h"
// math
#include "../math/lsVector3.h"
#include "../math/lsVector4.h"

// Forward declarations
class lsAnimNode;
class lsProperty;
class lsAnimProperty;
class lsAnimProperty_Vector3;
class lsAnimProperty_Vector4;
class lsBatch;

/**
 * @class lsAnimationManager
 * @brief Manages keyframe animations for nodes
 * 
 * Handles animation of:
 * - Transform: position, rotation, scale
 * - Material: diffuse, specular, ambient colors
 * 
 * @section playback Playback
 * 
 * - Play(loop): Start animation, optionally loop
 * - Stop(): Stop animation
 * - UpdateAll(): Advance animation by delta time
 * 
 * @section keys Keyframes
 * 
 * Keyframes are inserted at specific frames:
 * - InsertKey_Pos(): Position keyframe
 * - InsertKey_Rot(): Rotation keyframe
 * - InsertKey_Scale(): Scale keyframe
 * - InsertKey_Diffuse(): Diffuse color keyframe
 * - InsertKey_Specular(): Specular color keyframe
 * - InsertKey_Ambient(): Ambient color keyframe
 */
class lsAnimationManager
{
public:
    /**
     * @brief Default constructor
     */
	lsAnimationManager();
	
    /**
     * @brief Destructor
     */
	virtual ~lsAnimationManager();

    /**
     * @brief Initialize animation manager
     * 
     * @param n Animatable node
     * @param fps Frames per second
     */
    void Init( lsAnimNode *n, double fps );

    //=========================================================================
	// Playback
	//=========================================================================
	
    /**
     * @brief Update all animation properties
     * 
     * Advances animation by delta time and updates
     * all animated properties.
     * 
     * @param delta Time since last frame
     */
    void UpdateAll( double delta );
    
    /**
     * @brief Start playback
     * 
     * @param loop true to loop animation
     */
    void Play( bool loop );
    
    /**
     * @brief Stop playback
     */
    void Stop();

    //=========================================================================
	// Properties
	//=========================================================================
	
    /**
     * @brief Add property to animate
     * @param p Property to animate
     */
	void Add_AnimProperty( lsProperty *p );

    //=========================================================================
	// Transform Keys
	//=========================================================================
	
    /**
     * @brief Insert position keyframe
     * @param frame Frame number
     * @param pos Position vector
     */
	void InsertKey_Pos( double frame, const lsVector3& pos );
	
    /**
     * @brief Insert rotation keyframe
     * @param frame Frame number
     * @param rot Rotation vector (degrees)
     */
	void InsertKey_Rot( double frame, const lsVector3& rot );
	
    /**
     * @brief Insert scale keyframe
     * @param frame Frame number
     * @param sca Scale vector
     */
	void InsertKey_Scale( double frame, const lsVector3& sca );

    //=========================================================================
	// Material Keys
	//=========================================================================
	
    /**
     * @brief Insert diffuse color keyframe
     * @param frame Frame number
     * @param val RGBA color
     */
	void InsertKey_Diffuse( double frame, const lsVector4& val );
	
    /**
     * @brief Insert specular color keyframe
     * @param frame Frame number
     * @param val RGBA color
     */
	void InsertKey_Specular( double frame, const lsVector4& val );
	
    /**
     * @brief Insert ambient color keyframe
     * @param frame Frame number
     * @param val RGBA color
     */
	void InsertKey_Ambient( double frame, const lsVector4& val );

    //=========================================================================
	// Settings
	//=========================================================================
	
    /**
     * @brief Set animation framerate
     * @param val Frames per second
     */
	void setFramerate( double val )
	{
		framerate = val;
	}

    /**
     * @brief Get animation framerate
     * @return Frames per second
     */
	double getFramerate() const { return framerate; }

    /**
     * @brief Get current frame
     * @return Current frame number
     */
	double getCurr_frame() const { return curr_frame; }

private:
    //=========================================================================
	// Private Members
	//=========================================================================
	
    /** @brief Animation property list */
	lsAnimProperty *animproperty_list;
    
    /** @brief Parent animatable node */
	lsAnimNode *animnode;

	//=========================================================================
	// Animation State
	//=========================================================================
	
    /** @brief Frames per second */
	double framerate;
    
    /** @brief Current frame */
	double curr_frame;
    
    /** @brief First frame of animation */
	double start_frame;
    
    /** @brief Last frame of animation */
	double end_frame;
    
    /** @brief Loop flag */
	bool looping;
    
    /** @brief Pause flag */
    bool pause;

	//=========================================================================
	// Fast Lookup Properties
	//=========================================================================
	
    /** @brief Position animation curve */
	lsAnimProperty_Vector3 *position;
    
    /** @brief Rotation animation curve */
	lsAnimProperty_Vector3 *rotation;
    
    /** @brief Scale animation curve */
	lsAnimProperty_Vector3 *scale;
    
    /** @brief Diffuse color animation curve */
	lsAnimProperty_Vector4 *diffuse;
    
    /** @brief Specular color animation curve */
	lsAnimProperty_Vector4 *specular;
    
    /** @brief Ambient color animation curve */
	lsAnimProperty_Vector4 *ambient;

};

#endif // LSANIMATIONMANAGER_H
