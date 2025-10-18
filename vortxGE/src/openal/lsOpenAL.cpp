#include "lsOpenAL.h"

#include <vector>

lsOpenAL::lsOpenAL()
{
}

lsOpenAL::~lsOpenAL()
{
    cout << "lsOpenAL::~lsOpenAL() - Cleaning up OpenAL..." << endl;
    Cleanup();
}

bool lsOpenAL::Init()
{
    cout << "lsOpenAL::Init() - Initializing OpenAL..." << endl;
    
    m_device = alcOpenDevice(nullptr); // Open the default audio device
    if (!m_device)
    {
        cerr << "ERROR: Failed to open OpenAL device." << endl;
        return false;
    }
    ALC_RETURN_ON_ERROR(m_device, false); // Check for ALC errors after opening device

    m_context = alcCreateContext(m_device, nullptr); // Create an OpenAL context
    if (!m_context)
    {
        cerr << "ERROR: Failed to create OpenAL context." << endl;
        ALC_RETURN_ON_ERROR(m_device, false); // Check for ALC errors related to context creation
        alcCloseDevice(m_device); // Cleanup device if context creation failed
        m_device = nullptr;
        return false;
    }
    ALC_RETURN_ON_ERROR(m_device, false); // Check for ALC errors after context creation

    if (!alcMakeContextCurrent(m_context)) // Make the context current
    {
        cerr << "ERROR: Failed to make OpenAL context current." << endl;
        ALC_RETURN_ON_ERROR(m_device, false); // Check for ALC errors related to making context current
        alcDestroyContext(m_context); // Cleanup context if failed to make current
        m_context = nullptr;
        alcCloseDevice(m_device); // Cleanup device
        m_device = nullptr;
        return false;
    }
    ALC_RETURN_ON_ERROR(m_device, false); // Check for ALC errors after making context current

    // Get and print device information
    const ALCchar* deviceName = alcGetString(m_device, ALC_DEVICE_SPECIFIER);
    cout << "INFO: OpenAL Device: " << (deviceName ? deviceName : "Default") << endl;
	
    // Optional: Set listener defaults (can be overridden by lsListener)
    alListenerf(AL_GAIN, 1.0f);
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    ALfloat orientation[] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f }; // LookAt, Up
    alListenerfv(AL_ORIENTATION, orientation);

    AL_RETURN_ON_ERROR(false); // Check for AL errors after listener setup

    cout << "INFO: OpenAL initialized successfully." << endl;
    return true;
}

void lsOpenAL::Cleanup()
{
    if (m_context)
    {
        alcMakeContextCurrent(nullptr); // Detach context
        alcDestroyContext(m_context);
        m_context = nullptr;
    }
    if (m_device)
    {
        alcCloseDevice(m_device);
        m_device = nullptr;
    }
}

bool lsOpenAL::CheckALError(const string& filename, int line) const
{
    ALenum error = alGetError();
    if (error != AL_NO_ERROR)
    {
        string errorString;
        switch (error)
        {
            case AL_INVALID_NAME: errorString = "AL_INVALID_NAME"; break;
            case AL_INVALID_ENUM: errorString = "AL_INVALID_ENUM"; break;
            case AL_INVALID_VALUE: errorString = "AL_INVALID_VALUE"; break;
            case AL_INVALID_OPERATION: errorString = "AL_INVALID_OPERATION"; break;
            case AL_OUT_OF_MEMORY: errorString = "AL_OUT_OF_MEMORY"; break;
            default: errorString = "Unknown AL Error"; break;
        }
        cerr << "OpenAL Error: " << errorString << " at " << filename << ":" << line << endl;
        return true;
    }
    return false;
}

bool lsOpenAL::CheckALCError(ALCdevice* device, const string& filename, int line) const
{
    ALCenum error = alcGetError(device);
    if (error != ALC_NO_ERROR)
    {
        string errorString;
        switch (error)
        {
            case ALC_INVALID_DEVICE: errorString = "ALC_INVALID_DEVICE"; break;
            case ALC_INVALID_CONTEXT: errorString = "ALC_INVALID_CONTEXT"; break;
            case ALC_INVALID_ENUM: errorString = "ALC_INVALID_ENUM"; break;
            case ALC_INVALID_VALUE: errorString = "ALC_INVALID_VALUE"; break;
            case ALC_OUT_OF_MEMORY: errorString = "ALC_OUT_OF_MEMORY"; break;
            default: errorString = "Unknown ALC Error"; break;
        }
        cerr << "OpenALC Error: " << errorString << " at " << filename << ":" << line << endl;
        return true;
    }
    return false;
}
