#pragma once

#include "lsAnimNode.h"

#include <string>
#include <vector>
#include <AL/al.h>

class lsSpeaker : public lsAnimNode
{
public:
    lsSpeaker(lsNode* parent);
    virtual ~lsSpeaker();

    // Override from lsAnimNode
    void Move(lsScene* scn, lsRender* render, double delta) override;

    // Load WAV file into OpenAL buffer
    bool LoadWav(const string& filename);
    void UnloadWav();
	
    // Control playback
    void Play();
    void Pause();
    void Stop();
    bool IsPlaying() const;

    // Set properties
    void setPitch(float pitch);
    void setGain(float gain);
    void setLooping(bool loop);
    void setRolloffFactor(float rolloffFactor);
    void setReferenceDistance(float refDistance);
    void setMaxDistance(float maxDistance);

    // Save data
    const string& toString() override;

private:
    ALuint m_sourceId = 0;
    ALuint m_bufferId = 0;

    float m_pitch = 1.0f;
    float m_gain = 1.0f;
    bool m_loop = false;
    float m_rolloffFactor = 1.0f;
    float m_refDistance = 1.0f;
    float m_maxDistance = 100.0f;
    
    // Cleanup function for OpenAL resources (was missing definition)
    void Cleanup(); 

    // Helper for WAV loading
    bool loadWavData(const string& filename, ALenum* format, ALvoid** data, ALsizei* size, ALsizei* freq);
};
