/**
 * @file lsSpeaker.h
 * @brief Audio source for positional sound
 * 
 * Represents a 3D positional audio source using OpenAL. Can play
 * WAV audio files with spatial positioning relative to the listener.
 * 
 * @author vortxGE
 * @date 2024
 * @see lsListener
 * @see lsOpenAL
 */

#pragma once

#include "lsAnimNode.h"

#include <string>
#include <vector>
#include <AL/al.h>

/**
 * @class lsSpeaker
 * @brief 3D positional audio source
 * 
 * A speaker is an audio source that can be placed in 3D space.
 * It emits sound that can be positioned and attenuated based on
 * distance from the listener.
 * 
 * Features:
 * - WAV file loading
 * - Positional 3D audio
 * - Volume/gain control
 * - Pitch adjustment
 * - Looping playback
 * - Distance attenuation (rolloff)
 * 
 * @note Inherits from lsAnimNode, so it can be animated
 * 
 * @example
 * @code
 * lsSpeaker* sound = new lsSpeaker(player);
 * sound->LoadWav("sounds/explosion.wav");
 * sound->setLooping(false);
 * sound->setGain(0.8f);
 * sound->setRolloffFactor(2.0f);
 * sound->Play();
 * @endcode
 */
class lsSpeaker : public lsAnimNode
{
public:
	/**
	 * @brief Construct speaker
	 * @param parent Parent node in scene graph
	 */
    lsSpeaker(lsNode* parent);
    
    /// Destructor - releases OpenAL resources
    virtual ~lsSpeaker();

    //=========================================================================
    // lsAnimNode Override
    //=========================================================================
    
    /**
     * @brief Update speaker position each frame
     * 
     * @param scn Pointer to scene
     * @param render Pointer to renderer
     * @param delta Time since last frame
     */
    void Move(lsScene* scn, lsRender* render, double delta) override;

    //=========================================================================
    // Audio Loading
    //=========================================================================
    
    /**
     * @brief Load WAV audio file
     * 
     * Loads a WAV file into an OpenAL buffer for playback.
     * 
     * @param filename Path to WAV file
     * @return true if loaded successfully
     */
    bool LoadWav(const string& filename);
    
    /**
     * @brief Unload audio buffer
     * 
     * Frees the OpenAL buffer resources.
     */
    void UnloadWav();
	
    //=========================================================================
    // Playback Control
    //=========================================================================
    
    /// Start or resume playback
    void Play();
    
    /// Pause playback (retains position)
    void Pause();
    
    /// Stop playback (resets to beginning)
    void Stop();
    
    /**
     * @brief Check if currently playing
     * @return true if playing
     */
    bool IsPlaying() const;

    //=========================================================================
    // Audio Properties
    //=========================================================================
    
    /**
     * @brief Set playback pitch
     * 
     * @param pitch Pitch multiplier (1.0 = normal, 2.0 = octave up)
     */
    void setPitch(float pitch);
    
    /**
     * @brief Set volume
     * 
     * @param gain Volume (0.0 = silent, 1.0 = full)
     */
    void setGain(float gain);
    
    /**
     * @brief Set looping
     * 
     * @param loop true to loop audio
     */
    void setLooping(bool loop);
    
    /**
     * @brief Set rolloff factor
     * 
     * Controls how quickly volume decreases with distance.
     * Higher values = faster attenuation.
     * 
     * @param rolloffFactor Rolloff multiplier
     */
    void setRolloffFactor(float rolloffFactor);
    
    /**
     * @brief Set reference distance
     * 
     * Distance at which volume begins to attenuate.
     * 
     * @param refDistance Distance in world units
     */
    void setReferenceDistance(float refDistance);
    
    /**
     * @brief Set maximum distance
     * 
     * Distance at which volume stops decreasing.
     * 
     * @param maxDistance Distance in world units
     */
    void setMaxDistance(float maxDistance);

    //=========================================================================
    // Serialization
    //=========================================================================
    
    /**
     * @brief Serialize to string
     * @return String representation
     */
    const string& toString() override;

private:
    //=========================================================================
    // Private Members
    //=========================================================================
    
    ALuint m_sourceId = 0;	///< OpenAL source ID
    ALuint m_bufferId = 0;	///< OpenAL buffer ID

    float m_pitch = 1.0f;		///< Playback pitch
    float m_gain = 1.0f;		///< Volume
    bool m_loop = false;		///< Looping enabled
    float m_rolloffFactor = 1.0f;	///< Distance rolloff
    float m_refDistance = 1.0f;	///< Reference distance
    float m_maxDistance = 100.0f;	///< Max distance
    
    //=========================================================================
    // Private Methods
    //=========================================================================
    
    /**
     * @brief Clean up OpenAL resources
     */
    void Cleanup(); 

    /**
     * @brief Load raw WAV data
     * 
     * @param filename File path
     * @param format Output format
     * @param data Audio data
     * @param size Data size
     * @param freq Sample frequency
     * @return true on success
     */
    bool loadWavData(const string& filename, ALenum* format, ALvoid** data, ALsizei* size, ALsizei* freq);
};
