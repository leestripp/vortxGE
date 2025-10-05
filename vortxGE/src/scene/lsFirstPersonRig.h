#pragma once

#include "lsObject.h" 
#include "lsCamera.h"
#include "../physics/lsBulletObject.h"
#include <bullet/btBulletDynamicsCommon.h>

class lsScene;
class lsRender;
class lsAssetManager;
class lsBulletShape; // Forward declare lsBulletShape

class lsFirstPersonRig : public lsObject
{
public:
    lsFirstPersonRig(lsScene* scene, btDiscreteDynamicsWorld* dynamicsWorld);
    virtual ~lsFirstPersonRig();

    // Override from lsObject (which inherits from lsAnimNode)
    virtual void Move(lsScene* scn, lsRender* render, double delta) override;
    virtual void Optimize(lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render) override;
    virtual void Draw(lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render) override;

    // Player input handlers
    void HandleKeyboardInput(float delta, bool forward, bool backward, bool strafeLeft, bool strafeRight, bool crouch, bool jump);
    void HandleMouseLook(float delta, float delta_x, float delta_y);

    // Getters for external access (e.g., from DemoScene)
    lsCamera* getCamera() const { return m_camera; }

    // Save data
    virtual const std::string& toString() override;

private:
    lsCamera* m_camera;                // The first-person camera (child of this rig)
    lsBulletObject* m_bulletObject;    // Physics rigid body for the player (owned by this rig, also set to base class bulletobject pointer)

    // Bullet shapes for standing and crouching
    btCapsuleShape* m_standingShape;
    btCapsuleShape* m_crouchingShape;
	
	// Assets for shapes so they are managed by scene's asset manager (pointers to owned assets)
	lsAsset* m_standingShapeAsset;
	lsAsset* m_crouchingShapeAsset;

    // Movement parameters
    float m_moveSpeed;      // Base movement speed
    float m_sprintSpeed;    // Speed multiplier when sprinting
    float m_crouchSpeed;    // Speed multiplier when crouching
    float m_jumpForce;      // Upward impulse for jumping
    float m_mouseXSensitivity;	// rotation applies differently so we need to split these.
	float m_mouseYSensitivity;
	
    // State
    bool m_isCrouching = false;
    bool m_isJumping = false;
    float m_currentCapsuleHeight; // Current interpolated capsule height for graphics (Bullet uses fixed shapes)
    float m_standingCapsuleHeight; // Height of the cylinder part for standing shape
    float m_crouchingCapsuleHeight; // Height of the cylinder part for crouching shape
    float m_capsuleRadius; // Radius of the capsule (same for both standing/crouching)
    float m_cameraHeightOffset; // Camera's local Y offset from the rig's (capsule's) center
    float m_standingCameraOffset; // Camera Y offset when standing
    float m_crouchingCameraOffset; // Camera Y offset when crouching

    // Internal helper for updating bullet's capsule collision shape
    void updateCapsuleShape(float targetHeight);
};
