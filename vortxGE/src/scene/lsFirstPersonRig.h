#pragma once

#include "lsObject.h" 
#include "lsCamera.h"
#include "../physics/lsBulletObject.h"
#include "../physics/lsBulletWorld.h"

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
	void HandleKeyboardInput(bool forward, bool backward, bool strafeLeft, bool strafeRight, bool crouch, bool jump, bool sprint);
	void HandleMouseLook(float delta, float delta_x, float delta_y);
	
	// Getters for external access (e.g., from DemoScene)
	lsCamera* getCamera() const { return m_camera; }
	
	// Save data
	virtual const std::string& toString() override;
	
private:
	void checkGroundCollision();
	void updateCapsuleShape();
	
	lsCamera* m_camera;                // The first-person camera (child of this rig)
	lsBulletObject* m_bulletObject;    // Physics rigid body for the player (owned by this rig, also set to base class bulletobject pointer)
	
	// Bullet shapes for standing and crouching
	btCapsuleShape* m_standingShape;
	btCapsuleShape* m_crouchingShape;
	
	// Assets for shapes so they are managed by scene's asset manager (pointers to owned assets)
	lsAsset* m_standingShapeAsset;
	lsAsset* m_crouchingShapeAsset;
	
	// Movement parameters
	float m_walkSpeed;      // Walk movement speed
	float m_sprintSpeed;    // Sprint speed
	float m_crouchSpeed;    // Crouching speed
	float m_jumpForce;      // Upward impulse for jumping
	float m_mouseXSensitivity;	// rotation applies differently to X and Y
	float m_mouseYSensitivity;
	// Ground check
	bool m_isOnGround = false;			// Track if player is on ground
	float m_groundCheckDistance = 0.1f;	// Distance to check for ground
	btVector3 m_movement;		// Stores current movement vector for momentum preservation
	float m_airResistance;		// Onlly while not on the ground.
	
	// State
	bool m_isCrouching = false;
	bool m_isJumping = false;
	float m_currentCapsuleHeight; // Current interpolated capsule height for graphics (Bullet uses fixed shapes)
	float m_standingCapsuleHeight; // Height of the cylinder part for standing shape
	float m_crouchingCapsuleHeight; // Height of the cylinder part for crouching shape
	float m_capsuleRadius; // Radius of the capsule (same for both standing/crouching)
	
	// Camera interpolation
	float m_cameraHeightOffset; // Current camera Y offset from the rig's (capsule's) center
	float m_targetCameraHeightOffset; // Target camera Y offset for interpolation
	float m_cameraLerpSpeed; // Speed of camera height interpolation (0.0 to 1.0)
	
	// Standing and crouching camera offsets
	float m_standingCameraOffset; // Camera Y offset when standing
	float m_crouchingCameraOffset; // Camera Y offset when crouching
};
