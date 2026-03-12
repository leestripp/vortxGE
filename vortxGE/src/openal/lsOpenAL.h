/**
 * @file lsOpenAL.h
 * @brief OpenAL audio system wrapper
 * 
 * Provides initialization and error checking for OpenAL audio.
 * This is the main entry point for the audio subsystem.
 * 
 * @author vortxGE
 * @date 2024
 * @see lsSpeaker
 * @see lsListener
 */

#pragma once

#include <iostream>
#include <string>
#include <AL/al.h>
#include <AL/alc.h>

using namespace std;

/**
 * @class lsOpenAL
 * @brief OpenAL audio system wrapper
 * 
 * Manages the OpenAL audio context and provides error checking
 * utilities. This is the core audio system class that must be
 * initialized before using any audio features.
 * 
 * @note There should typically be only one instance of this class
 * 
 * @example
 * @code
 * lsOpenAL* audio = new lsOpenAL();
 * if (audio->Init()) {
 *     // Audio system ready
 *     lsListener* listener = new lsListener(camera);
 *     lsSpeaker* sound = new lsSpeaker(object);
 *     sound->LoadWav("music.wav");
 *     sound->Play();
 * }
 * // On shutdown
 * audio->Cleanup();
 * @endcode
 */
class lsOpenAL
{
public:
	/// Default constructor
	lsOpenAL();
	
	/// Destructor - calls Cleanup()
	virtual ~lsOpenAL();

	/**
	 * @brief Initialize OpenAL audio system
	 * 
	 * Creates the audio context and makes it current.
	 * Must be called before any other audio operations.
	 * 
	 * @return true if initialization successful
	 */
	bool Init();
	
	/// Shutdown OpenAL and release resources
	void Cleanup();

	/**
	 * @brief Get audio device
	 * @return OpenAL device handle
	 */
	ALCdevice* getDevice() const { return m_device; }
	
	/**
	 * @brief Get audio context
	 * @return OpenAL context handle
	 */
	ALCcontext* getContext() const { return m_context; }

	//=========================================================================
	// Error Checking
	//=========================================================================
	
	/**
	 * @brief Check for OpenAL errors
	 * 
	 * @param filename Source file for error reporting
	 * @param line Line number for error reporting
	 * @return true if error occurred
	 */
	bool CheckALError(const string& filename, int line) const;
	
	/**
	 * @brief Check for OpenAL device errors
	 * 
	 * @param device Device to check
	 * @param filename Source file for error reporting
	 * @param line Line number for error reporting
	 * @return true if error occurred
	 */
	bool CheckALCError(ALCdevice* device, const string& filename, int line) const;

private:
	ALCdevice* m_device = nullptr;	///< OpenAL audio device
	ALCcontext* m_context = nullptr;	///< OpenAL context
};

//=========================================================================
// Helper Macros
//=========================================================================

/**
 * @brief Macro for OpenAL error checking
 * 
 * Executes an OpenAL function and checks for errors.
 * Requires m_openal pointer to be available.
 * 
 * @example
 * @code
 * AL_CHECK(alSourcePlay(sourceId));
 * @endcode
 */
#define AL_CHECK(al_function) \
    al_function; \
    if (m_openal && m_openal->CheckALError(__FILE__, __LINE__)) { /* Error occurred */ }

/**
 * @brief Macro for OpenAL device error checking
 * 
 * Executes an OpenALC function and checks for errors.
 * 
 * @example
 * @code
 * ALC_CHECK(alcMakeContextCurrent(context), device);
 * @endcode
 */
#define ALC_CHECK(alc_function, device) \
    alc_function; \
    if (m_openal && m_openal->CheckALCError(device, __FILE__, __LINE__)) { /* Error occurred */ }

/**
 * @brief Macro to return on OpenAL error
 * 
 * Checks for error after an OpenAL call and returns
 * specified value if error occurred. Use inside lsOpenAL methods.
 */
#define AL_RETURN_ON_ERROR(return_value) \
    if (this->CheckALError(__FILE__, __LINE__)) { return return_value; }

/**
 * @brief Macro to return on OpenALC error
 * 
 * Checks for error after an OpenALC call and returns
 * specified value if error occurred.
 */
#define ALC_RETURN_ON_ERROR(device, return_value) \
    if (this->CheckALCError(device, __FILE__, __LINE__)) { return return_value; }
