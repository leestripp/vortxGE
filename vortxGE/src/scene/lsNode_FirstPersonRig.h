#pragma once

#include "lsObject.h"
#include <algorithm> // For std::clamp
#include <LinearMath/btVector3.h> // For btVector3

// Forward declarations
class lsCamera;
class lsBulletObject;
class lsAsset_BulletShape_Pill;

class lsNode_FirstPersonRig : public lsObject
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
    void updateCapsuleShape();

    lsCamera* m_camera;
    lsBulletObject* m_bulletObject;
    lsAsset_BulletShape_Pill* m_standShapeAsset;
    lsAsset_BulletShape_Pill* m_crouchShapeAsset;

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
    bool m_wasCrouching = false; // To detect state changes
    bool m_wasOnGround = true;   // To detect leaving/landing on ground
    btVector3 m_airborneHorizontalVelocity; // To preserve momentum while airborne

    // Movement parameters
    const float m_walkSpeed = 5.0f;
    const float m_sprintSpeed = 8.0f;
    const float m_crouchSpeed = 2.5f;
    const float m_jumpSpeed = 14.0f;
    const float m_mouseSensitivity = 0.1f;
	
    // Camera and capsule dimensions
    const float m_capsuleRadius = 0.5f;
    const float m_standingCapsuleHeight = 1.0f;
    const float m_crouchingCapsuleHeight = 0.2f;
    const float m_standingCamHeight = 0.5f;   // Relative to rig's center
    const float m_crouchedCamHeight = 0.0f;   // Relative to rig's center
};
