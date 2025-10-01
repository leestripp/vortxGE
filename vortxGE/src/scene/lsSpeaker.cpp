#include "lsSpeaker.h"

#include "lsScene.h"
#include "../render/lsOpenGL.h"
#include "../filesystem/lsFileIO.h"
#include "../filesystem/lsDir.h"
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <iostream> // Included for std::cerr/std::cout

lsSpeaker::lsSpeaker(lsNode* parent) : lsAnimNode(parent)
{
    setNode_type(LSNT_NODE); // Speaker doesn't have its own distinct LSNT_ type, use NODE
    setName("AudioSpeaker");

    // Generate OpenAL source
    alGenSources(1, &m_sourceId);
    if (m_sourceId == 0) {
        std::cerr << "ERROR: Failed to generate OpenAL source for speaker: " << getName() << std::endl;
        return;
    }

    // Set default properties
    alSourcef(m_sourceId, AL_PITCH, m_pitch);
    alSourcef(m_sourceId, AL_GAIN, m_gain);
    alSourcei(m_sourceId, AL_LOOPING, m_loop ? AL_TRUE : AL_FALSE);
    alSourcef(m_sourceId, AL_ROLLOFF_FACTOR, m_rolloffFactor);
    alSourcef(m_sourceId, AL_REFERENCE_DISTANCE, m_refDistance);
    alSourcef(m_sourceId, AL_MAX_DISTANCE, m_maxDistance);
    alSource3f(m_sourceId, AL_POSITION, position.x, position.y, position.z);
    alSource3f(m_sourceId, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
}

// Implementation for the Cleanup function that was called in the destructor
void lsSpeaker::Cleanup()
{
    // Stop the source if it's currently playing
    alSourceStop(m_sourceId);
    
    // Detach and delete the buffer
    UnloadWav();

    // Delete the source
    if (m_sourceId != 0) {
        alDeleteSources(1, &m_sourceId);
        m_sourceId = 0;
    }
}

lsSpeaker::~lsSpeaker()
{
    std::cout << "lsSpeaker::~lsSpeaker() - Cleaning up OpenAL..." << std::endl;
    Cleanup();
}

void lsSpeaker::Move(lsScene* scn, lsOpenGL* opengl, double delta)
{
    // First, call the base class Move to update the ModelMatrix
    lsAnimNode::Move(scn, opengl, delta);

    // Update OpenAL source position based on the ModelMatrix
    if (m_sourceId != 0) {
        alSource3f(m_sourceId, AL_POSITION,
                   ModelMatrix.matrix[12], ModelMatrix.matrix[13], ModelMatrix.matrix[14]);
        // For velocity, a proper calculation would involve tracking previous position
        alSource3f(m_sourceId, AL_VELOCITY, 0.0f, 0.0f, 0.0f); // Simplistic zero velocity
    }
}

bool lsSpeaker::LoadWav(const std::string& filename)
{
    UnloadWav(); // Unload any existing buffer first

    ALenum format;
    ALvoid* data = nullptr;
    ALsizei size;
    ALsizei freq;
	
    if (!loadWavData(filename, &format, &data, &size, &freq))
    {
        std::cerr << "ERROR: Failed to load WAV data from " << filename << std::endl;
        return false;
    }
	
    // Generate OpenAL buffer
    alGenBuffers(1, &m_bufferId);
    if (m_bufferId == 0)
    {
        std::cerr << "ERROR: Failed to generate OpenAL buffer for " << filename << std::endl;
        free(data);
        return false;
    }
	
    // Bind WAV data to buffer
    alBufferData(m_bufferId, format, data, size, freq);
    free(data); // Data copied, can free now

    // Attach buffer to source
    alSourcei(m_sourceId, AL_BUFFER, m_bufferId);

    std::cout << "INFO: Loaded WAV file: " << filename << std::endl;
    return true;
}

void lsSpeaker::UnloadWav()
{
    if (m_bufferId != 0) {
        alSourcei(m_sourceId, AL_BUFFER, 0); // Detach buffer from source
        alDeleteBuffers(1, &m_bufferId);
        m_bufferId = 0;
    }
}

void lsSpeaker::Play()
{
    if (m_sourceId != 0 && m_bufferId != 0) {
        alSourcePlay(m_sourceId);
    }
}

void lsSpeaker::Pause()
{
    if (m_sourceId != 0) {
        alSourcePause(m_sourceId);
    }
}

void lsSpeaker::Stop()
{
    if (m_sourceId != 0) {
        alSourceStop(m_sourceId);
    }
}

bool lsSpeaker::IsPlaying() const
{
    if (m_sourceId == 0) return false;
    ALint state;
    alGetSourcei(m_sourceId, AL_SOURCE_STATE, &state);
    return (state == AL_PLAYING);
}

void lsSpeaker::setPitch(float pitch)
{
    m_pitch = pitch;
    alSourcef(m_sourceId, AL_PITCH, m_pitch);
}

void lsSpeaker::setGain(float gain)
{
    m_gain = gain;
    alSourcef(m_sourceId, AL_GAIN, m_gain);
}

void lsSpeaker::setLooping(bool loop)
{
    m_loop = loop;
    alSourcei(m_sourceId, AL_LOOPING, m_loop ? AL_TRUE : AL_FALSE);
}

void lsSpeaker::setRolloffFactor(float rolloffFactor)
{
    m_rolloffFactor = rolloffFactor;
    alSourcef(m_sourceId, AL_ROLLOFF_FACTOR, m_rolloffFactor);
}

void lsSpeaker::setReferenceDistance(float refDistance)
{
    m_refDistance = refDistance;
    alSourcef(m_sourceId, AL_REFERENCE_DISTANCE, m_refDistance);
}

void lsSpeaker::setMaxDistance(float maxDistance)
{
    m_maxDistance = maxDistance;
    alSourcef(m_sourceId, AL_MAX_DISTANCE, m_maxDistance);
}

bool lsSpeaker::loadWavData(const std::string& filename, ALenum* format, ALvoid** data, ALsizei* size, ALsizei* freq)
{
    std::string fullPath = lsDir().getexepath() + filename;
    std::ifstream file(fullPath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "ERROR: Failed to open WAV file: " << fullPath << std::endl;
        return false;
    }

    char chunkID[4];
    uint32_t chunkSize;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint16_t bitsPerSample;
    uint32_t dataSize = 0;

    // RIFF chunk
    file.read(chunkID, 4);
    if (std::string(chunkID, 4) != "RIFF") {
        std::cerr << "ERROR: Invalid WAV header: 'RIFF' chunk missing." << std::endl;
        file.close();
        return false;
    }
    file.read(reinterpret_cast<char*>(&chunkSize), 4); // File size
    file.read(chunkID, 4); // "WAVE"
    if (std::string(chunkID, 4) != "WAVE") {
        std::cerr << "ERROR: Invalid WAV header: 'WAVE' chunk missing." << std::endl;
        file.close();
        return false;
    }

    // Iterate through chunks until "fmt " and "data" are found
    bool fmt_found = false;
    bool data_found = false;

    while (file.read(chunkID, 4)) {
        file.read(reinterpret_cast<char*>(&chunkSize), 4);

        if (file.eof()) {
            std::cerr << "ERROR: Unexpected end of WAV file while reading chunks." << std::endl;
            file.close();
            return false;
        }

        std::string sChunkID(chunkID, 4);

        if (sChunkID == "fmt ") {
            fmt_found = true;
            file.read(reinterpret_cast<char*>(&audioFormat), 2);
            file.read(reinterpret_cast<char*>(&numChannels), 2);
            file.read(reinterpret_cast<char*>(&sampleRate), 4);
            file.seekg(4, std::ios_base::cur); // Skip ByteRate
            file.seekg(2, std::ios_base::cur); // Skip BlockAlign
            file.read(reinterpret_cast<char*>(&bitsPerSample), 2);

            // Skip any extra bytes in fmt chunk if present
            if (chunkSize > 16) {
                file.seekg(chunkSize - 16, std::ios_base::cur);
            }
        } else if (sChunkID == "data") {
            data_found = true;
            dataSize = chunkSize;
            break; // Found data chunk, stop iterating
        } else {
            // Skip unknown chunk
            file.seekg(chunkSize, std::ios_base::cur);
        }
    }

    if (!fmt_found) {
        std::cerr << "ERROR: WAV 'fmt ' chunk not found." << std::endl;
        file.close();
        return false;
    }
    if (!data_found) {
        std::cerr << "ERROR: WAV 'data' chunk not found." << std::endl;
        file.close();
        return false;
    }

    // Validate audio format
    if (audioFormat != 1) { // Only PCM is supported
        std::cerr << "ERROR: WAV audio format is not PCM. (AudioFormat = " << audioFormat << ")" << std::endl;
        file.close();
        return false;
    }

    // Determine OpenAL format
    if (numChannels == 1 && bitsPerSample == 8) {
        *format = AL_FORMAT_MONO8;
    } else if (numChannels == 1 && bitsPerSample == 16) {
        *format = AL_FORMAT_MONO16;
    } else if (numChannels == 2 && bitsPerSample == 8) {
        *format = AL_FORMAT_STEREO8;
    } else if (numChannels == 2 && bitsPerSample == 16) {
        *format = AL_FORMAT_STEREO16;
    } else {
        std::cerr << "ERROR: Unsupported WAV format: " << numChannels << " channels, " << bitsPerSample << " bits per sample." << std::endl;
        file.close();
        return false;
    }

    *size = dataSize;
    *freq = sampleRate;

    *data = malloc(dataSize);
    if (!*data) {
        std::cerr << "ERROR: Out of memory for WAV data buffer (size: " << dataSize << " bytes)." << std::endl;
        file.close();
        return false;
    }
    file.read(reinterpret_cast<char*>(*data), dataSize);

    file.close();
    return true;
}

const std::string& lsSpeaker::toString()
{
    save_str = "add_speaker ";
    if (getName() != "")
    {
        save_str += "`";
        save_str += getName();
        save_str += "`";
    }
    else
    {
        save_str += "`AudioSpeaker`";
    }

    if (getParent())
    {
        save_str += " `";
        save_str += getParent()->getName();
        save_str += "`";
    }
    else
    {
        save_str += " `NONE`";
    }

    save_str += " " + position.toString();
    save_str += " " + rotation.toString();
    save_str += " " + scale.toString();
    
    return save_str;
}

