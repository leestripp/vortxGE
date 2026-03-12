/**
 * @file lsListener.h
 * @brief Audio listener representing the player's ears
 * 
 * The listener represents the position and orientation of the player's
 * ears in 3D space. All positional audio is calculated relative to
 * the listener's position.
 * 
 * @author vortxGE
 * @date 2024
 * @see lsSpeaker
 * @see lsOpenAL
 */

#pragma once

#include "lsAnimNode.h"
#include <AL/al.h>

/**
 * @class lsListener
 * @brief Audio listener (player's ears)
 * 
 * The listener represents the point from which all audio is heard.
 * It has position and orientation that affects how sounds from
 * lsSpeaker objects are perceived.
 * 
 * Typically, there is one listener per scene, usually attached
 * to the camera or player object.
 * 
 * @note Inherits from lsAnimNode, so position is updated automatically
 * 
 * @example
 * @code
 * lsListener* listener = new lsListener(camera);
 * listener->setGain(1.0f);
 * listener->setOrientation(0, 0, -1, 0, 1, 0); // Looking down -Z, up is +Y
 * @endcode
 */
class lsListener : public lsAnimNode
{
public:
	/**
	 * @brief Construct listener
	 * @param parent Parent node (usually camera)
	 */
    lsListener(lsNode* parent);
    
    /// Destructor
    virtual ~lsListener();
	
    //=========================================================================
    // lsAnimNode Override
    //=========================================================================
    
    /**
     * @brief Update listener position each frame
     * 
     * @param scn Pointer to scene
     * @param render Pointer to renderer
     * @param delta Time since last frame
     */
    void Move(lsScene* scn, lsRender* render, double delta) override;
    
    //=========================================================================
    // Listener Properties
    //=========================================================================
    
    /**
     * @brief Set overall listener gain
     * 
     * Master volume for all audio.
     * 
     * @param gain Volume (0.0 = silent, 1.0 = full)
     */
    void setGain(float gain);
    
    /**
     * @brief Set listener velocity
     * 
     * Used for Doppler effect calculations.
     * 
     * @param x Velocity X
     * @param y Velocity Y
     * @param z Velocity Z
     */
    void setVelocity(float x, float y, float z);
    
    /**
     * @brief Set listener orientation
     * 
     * Defines the "at" vector (where listener is facing) and
     * "up" vector (listener's up direction).
     * 
     * @param at_x "At" vector X (forward)
     * @param at_y "At" vector Y
     * @param at_z "At" vector Z
     * @param up_x "Up" vector X
     * @param up_y "Up" vector Y
     * @param up_z "Up" vector Z
     */
    void setOrientation(float at_x, float at_y, float at_z, float up_x, float up_y, float up_z);

    //=========================================================================
    // Serialization
    //=========================================================================
    
    /**
     * @brief Serialize to string
     * @return String representation
     */
    const string& toString() override;

private:
    float m_gain = 1.0f;	///< Master volume
    
    /// Orientation: [0-2] = at vector, [3-5] = up vector
    ALfloat m_orientation[6] = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f}; 
};
