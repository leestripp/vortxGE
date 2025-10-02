#include "lsListener.h"

#include "lsScene.h"
#include "../render/lsRender.h"
#include "../math/lsMatrix4x4.h"

lsListener::lsListener(lsNode* parent) : lsAnimNode(parent)
{
    setNode_type(LSNT_NODE); // Listener doesn't have its own distinct LSNT_ type, use NODE
    setName("AudioListener");
    
    // Default position/rotation for a listener. Will be updated by Move.
    Set_Pos(0, 0, 0);
    Set_Rot(0, 0, 0);
    Set_Scale(1.0f, 1.0f, 1.0f);
}

lsListener::~lsListener()
{
}

void lsListener::Move(lsScene* scn, lsRender* render, double delta)
{
    // First, call the base class Move to update the ModelMatrix
    lsAnimNode::Move(scn, render, delta);

    // Now, update the OpenAL listener position and orientation based on the ModelMatrix
    
    // Position is the translation component of the ModelMatrix
    ALfloat listenerPos[] = {
        ModelMatrix.matrix[12],
        ModelMatrix.matrix[13],
        ModelMatrix.matrix[14]
    };
    alListenerfv(AL_POSITION, listenerPos);

    // The 'at' vector is the forward direction (negative Z-axis in local space)
    // The 'up' vector is the positive Y-axis in local space
    // These need to be transformed into world space (rotation only)
    
    // Extract rotation part of the ModelMatrix
    lsMatrix4x4 rotationMatrix = ModelMatrix.extractRotation();

    // Local forward (-Z) vector
    lsVector3 localForward(0.0f, 0.0f, -1.0f);
    // Local up (+Y) vector
    lsVector3 localUp(0.0f, 1.0f, 0.0f);

    // Transform to world space
    lsVector3 worldForward = rotationMatrix.Rotate(localForward);
    lsVector3 worldUp = rotationMatrix.Rotate(localUp);

    m_orientation[0] = worldForward.x;
    m_orientation[1] = worldForward.y;
    m_orientation[2] = worldForward.z;
    m_orientation[3] = worldUp.x;
    m_orientation[4] = worldUp.y;
    m_orientation[5] = worldUp.z;
    
    alListenerfv(AL_ORIENTATION, m_orientation);

    // For simplicity, velocity is set to zero for now.
    // In a real game, you would calculate this from previous vs current position.
    alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
}

void lsListener::setGain(float gain)
{
    m_gain = gain;
    alListenerf(AL_GAIN, m_gain);
}

void lsListener::setVelocity(float x, float y, float z)
{
    alListener3f(AL_VELOCITY, x, y, z);
}

void lsListener::setOrientation(float at_x, float at_y, float at_z, float up_x, float up_y, float up_z)
{
    m_orientation[0] = at_x;
    m_orientation[1] = at_y;
    m_orientation[2] = at_z;
    m_orientation[3] = up_x;
    m_orientation[4] = up_y;
    m_orientation[5] = up_z;
    alListenerfv(AL_ORIENTATION, m_orientation);
}

const std::string& lsListener::toString()
{
    save_str = "add_listener ";
    if (getName() != "")
    {
        save_str += "`";
        save_str += getName();
        save_str += "`";
    }
    else
    {
        save_str += "`AudioListener`";
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
