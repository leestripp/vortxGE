#pragma once

#include "lsAnimNode.h"
#include <AL/al.h>

class lsListener : public lsAnimNode
{
public:
    lsListener(lsNode* parent);
    virtual ~lsListener();
	
    // Override from lsAnimNode
    void Move(lsScene* scn, lsOpenGL* opengl, double delta) override;
    
    // Set OpenAL listener properties
    void setGain(float gain);
    void setVelocity(float x, float y, float z);
    void setOrientation(float at_x, float at_y, float at_z, float up_x, float up_y, float up_z);

    // Save data
    const std::string& toString() override;

private:
    float m_gain = 1.0f;
    ALfloat m_orientation[6] = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f}; // Listen At vector, Up vector
};
