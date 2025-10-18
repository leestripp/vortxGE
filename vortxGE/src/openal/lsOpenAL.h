#pragma once

#include <iostream>
#include <string>
#include <AL/al.h>
#include <AL/alc.h>

using namespace std;

class lsOpenAL
{
public:
	lsOpenAL();
	virtual ~lsOpenAL();

	bool Init();
	void Cleanup();

	ALCdevice* getDevice() const { return m_device; }
	ALCcontext* getContext() const { return m_context; }

	// Error checking
	bool CheckALError(const string& filename, int line) const;
	bool CheckALCError(ALCdevice* device, const string& filename, int line) const;

private:
	ALCdevice* m_device = nullptr;
	ALCcontext* m_context = nullptr;
};

// Macro for easier error checking when an lsOpenAL* m_openal pointer is available (e.g., in other classes)
// These macros execute the OpenAL/OpenALC function and then check for errors.
#define AL_CHECK(al_function) \
    al_function; \
    if (m_openal && m_openal->CheckALError(__FILE__, __LINE__)) { /* Error occurred */ }

#define ALC_CHECK(alc_function, device) \
    alc_function; \
    if (m_openal && m_openal->CheckALCError(device, __FILE__, __LINE__)) { /* Error occurred */ }

// Macros for use *inside* lsOpenAL methods for immediate return on error.
// These assume 'this' is available and are used *after* an OpenAL/OpenALC call
// to check the error state and return a specified value if an error occurred.
#define AL_RETURN_ON_ERROR(return_value) \
    if (this->CheckALError(__FILE__, __LINE__)) { return return_value; }

#define ALC_RETURN_ON_ERROR(device, return_value) \
    if (this->CheckALCError(device, __FILE__, __LINE__)) { return return_value; }
