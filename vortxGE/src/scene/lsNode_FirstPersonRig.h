
#pragma once

#include "lsAnimNode.h"
#include <algorithm> // For std::clamp

// Forward declarations
class lsCamera;
class lsBulletObject;
class lsAsset_BulletShape_Pill;

class lsNode_FirstPersonRig : public lsAnimNode
{
public:
    lsNode_FirstPersonRig(lsNode* parent);
    virtual ~lsNode_FirstPersonRig();

    void Move(lsScene* scn, lsOpenGL* opengl, double delta) override;
    void Optimize(lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl) override;
    const string& toString() override;

    void handleInput(bool forward, bool backward, bool left, bool right, bool jump, bool sprint, bool crouch);
    void handleMouseLook(float deltaYaw, float deltaPitch);

    lsCamera* getCamera() const { return m_camera; }

private:
    bool onGround();

    lsCamera* m_camera;
    lsBulletObject* m_bulletObject;
    lsAsset_BulletShape_Pill* m_shapeAsset;

    float m_yaw = 0.0f;
    float m_pitch = 0.0f;

    // Movement state
    bool m_moveForward = false;
    bool m_moveBackward = false;
    bool m_strafeLeft = false;
    bool m_strafeRight = false;
    bool m_wantsToJump = false;
    bool m_isSprinting = false;
    bool m_isCrouching = false;

    // Movement parameters
    float m_walkSpeed = 5.0f;
    float m_sprintSpeed = 8.0f;
    float m_crouchSpeed = 2.5f;
    float m_jumpForce = 7.0f;
    float m_mouseSensitivity = 0.1f;

    // Camera and capsule dimensions
    float m_crouchedCamHeight = 0.4f; // Relative to rig's center
    float m_standingCamHeight = 0.5f; // Relative to rig's center (Total height 2, camera at 1.5 from floor)
    float m_capsuleHeight = 1.0f;     // The cylindrical part of the capsule
};
