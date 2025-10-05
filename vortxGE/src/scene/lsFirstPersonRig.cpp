#include "lsFirstPersonRig.h"

#include "lsScene.h"
#include "../render/lsRender.h"
#include "../assets/lsAssetManager.h"
#include "../assets/lsAsset_BulletShape_Pill.h" // For capsule shapes
#include "../datablocks/lsBulletShape.h"       // To get btCollisionShape* from lsBulletShape
#include "../math/lsQuaternion.h"              // For rotation calculations

// physics
#include "../physics/lsBulletWorld.h"
#include "../physics/lsBulletObject.h"

#include <iostream>
#include <algorithm> // For std::clamp

lsFirstPersonRig::lsFirstPersonRig(lsScene* scene, btDiscreteDynamicsWorld* dynamicsWorld)
	: lsObject(scene), // Call base class constructor with the scene as parent
	  m_camera(nullptr),
	  m_bulletObject(nullptr),
	  m_standingShape(nullptr),
	  m_crouchingShape(nullptr),
	  m_standingShapeAsset(nullptr),
	  m_crouchingShapeAsset(nullptr)
{
	setNode_type(LSNT_FIRSTPERSONRIG);
	setName("FirstPersonRig");

	// Default rig properties
	Set_Pos(0, 5, 0); // Start slightly above ground
	Set_Rot(0, 0, 0);
	Set_Scale(1.0f, 1.0f, 1.0f);
	
	// init movement
	m_movement = btVector3(0, 0, 0);
	
	// Movement parameters
	m_walkSpeed = 4.0f;
	m_sprintSpeed = 8.0f;
	m_crouchSpeed = 2.0f;
	m_jumpForce = 15.0f;
	m_mouseXSensitivity = 0.05f;
	m_mouseYSensitivity = 1.5f;
	m_airResistance = 0.95;		// Slows down the player if not on the ground.
	
	// Capsule dimensions
	m_capsuleRadius = 0.5f;
	m_standingCapsuleHeight = 1.8f; // Total height 1.8m
	m_crouchingCapsuleHeight = 1.0f; // Total height 1.0m
	
	// Camera offset from capsule center
	// Camera is roughly at eye level, so slightly above the center of the capsule.
	// Assuming capsule origin is at its center of mass.
	m_standingCameraOffset = 0.6f; // For a 1.8m capsule, COG is at 0, eyes are at +0.6, total height = radius + cylinderHeight/2 + offset
	m_crouchingCameraOffset = 0.0f; // For a 1.0m capsule, COG is at 0, eyes are at 0 (or slight offset)
	
	m_currentCapsuleHeight = m_standingCapsuleHeight; // Start standing
	m_cameraHeightOffset = m_standingCameraOffset;
	
	// Create bullet shapes using the scene's asset manager
	lsAssetManager* am = scene->getAssetManager();
	if (!am)
	{
		std::cerr << "ERROR: lsFirstPersonRig - AssetManager not available from scene." << std::endl;
		return;
	}

	// Standing capsule shape
	m_standingShapeAsset = am->CreateAsset_BulletShape_Pill(m_capsuleRadius, m_standingCapsuleHeight - 2 * m_capsuleRadius);
	if (m_standingShapeAsset)
	{
		m_standingShapeAsset->AllocAsset();
		lsBulletShape* bs = static_cast<lsBulletShape*>(m_standingShapeAsset->getData());
		m_standingShape = static_cast<btCapsuleShape*>(bs->getShape());
		
	} else
	{
		std::cerr << "ERROR: lsFirstPersonRig - Failed to create standing capsule asset." << std::endl;
	}
	
	// Crouching capsule shape
	m_crouchingShapeAsset = am->CreateAsset_BulletShape_Pill(m_capsuleRadius, m_crouchingCapsuleHeight - 2 * m_capsuleRadius);
	if (m_crouchingShapeAsset)
	{
		m_crouchingShapeAsset->AllocAsset();
		lsBulletShape* bs = static_cast<lsBulletShape*>(m_crouchingShapeAsset->getData());
		m_crouchingShape = static_cast<btCapsuleShape*>(bs->getShape());
	} else
	{
		std::cerr << "ERROR: lsFirstPersonRig - Failed to create crouching capsule asset." << std::endl;
	}
	
	// --- In the constructor, after setting up physics ---
	if (m_standingShape)
	{
		m_bulletObject = new lsBulletObject(this, m_standingShape, scene->getBulletworld()->getMax_proxies());
		if (m_bulletObject)
		{
			// Set mass for dynamic object
			m_bulletObject->Init(75.0f); // ~75 kg player
			
			// Configure physics properties for better sliding
			btRigidBody* body = m_bulletObject->getBody();
			body->setSleepingThresholds(0.0f, 0.0f); // Prevent sleeping
			body->setAngularFactor(btVector3(0.0f, 1.0f, 0.0f)); // Only allow yaw rotation via physics
			body->setDamping(0.9f, 0.9f); // Linear and angular damping
			
			// Set friction and restitution for sliding behavior
			body->setFriction(0.1f); // Low friction for sliding
			body->setRollingFriction(0.1f);
			body->setSpinningFriction(0.1f);
			body->setRestitution(0.0f); // No bounce
			
			body->setGravity(dynamicsWorld->getGravity()); // Inherit world gravity
			
			setBulletObject(m_bulletObject);
		} else
		{
			std::cerr << "ERROR: lsFirstPersonRig - Failed to create BulletObject." << std::endl;
		}
	} else
	{
		std::cerr << "ERROR: lsFirstPersonRig - Standing capsule shape is NULL, cannot initialize BulletObject." << std::endl;
	}
	
	// Create camera and attach as child
	m_camera = new lsCamera(this);
	if (m_camera)
	{
		// Position camera relative to the rig's center
		// The camera's local position will be adjusted in Move for crouch/stand
		m_camera->Set_Pos(0.0f, m_cameraHeightOffset, 0.0f);
		m_camera->Set_Rot(0.0f, 0.0f, 0.0f); // Camera rotation is relative to rig
		m_camera->Set_Perspective(60.0f, 1920.0f / 1080.0f, 0.1f, 1000.0f);
		
		Add_Node(m_camera); // Add camera as a child node
	} else
	{
		std::cerr << "ERROR: lsFirstPersonRig - Failed to create camera." << std::endl;
	}
}

lsFirstPersonRig::~lsFirstPersonRig()
{
	// The base class `lsObject` destructor will handle `delete bulletobject;`
	// and `Del_Children()` will handle `delete m_camera;` since it's stored as unique_ptr.
	// Asset shapes are managed by the scene's asset manager, so we don't delete them here.
}

void lsFirstPersonRig::Move(lsScene* scn, lsRender* render, double delta)
{
	// First, call the base class Move.
	// This updates ModelMatrix based on physics (if m_bulletObject is linked to lsObject::bulletobject).
	// It also handles animation for the rig itself (pos/rot/scale).
	lsObject::Move(scn, render, delta);
	
	// Update camera's local position for crouching/standing
	// The m_camera is a child, so its local position will be added to the rig's world position.
	m_camera->Set_Pos(0.0f, m_cameraHeightOffset, 0.0f);
	
	// Ensure the physics body's position matches the rig's (base object's) position
	// This is important if something external changes the rig's position (e.g., scene loading)
	// or if the rig's position is manually set without using Set_Pos (which updates bullet).
	// The bulletobject->Apply_Physics() call in lsObject::Move() copies the physics state
	// to ModelMatrix, so this ensures consistency for rendering.
	// If we want to move the rig and NOT have physics react, we'd adjust ModelMatrix directly
	// AFTER the physics update. For FPS, physics dictates rig position.

	// No need to update m_bulletObject->Set_Position() here explicitly
	// because lsObject::Move() already updates ModelMatrix from physics.
	// We update m_isCrouching state based on input, and updateCapsuleShape()
	// ensures the bullet collision shape reflects that, affecting future physics.
}

void lsFirstPersonRig::Optimize(lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render)
{
	// Pass optimization calls to the camera as it handles the actual view frustum
	if (m_camera) {
		m_camera->Optimize(cam, pass, scn, render);
	}
	// The rig itself typically doesn't need complex optimization separate from its camera
}

void lsFirstPersonRig::Draw(lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render)
{
	// The rig itself usually has no visual representation, but its camera does.
	// No explicit drawing for the rig. Drawing for the camera is handled by the scene's passes.
	// If you wanted to draw a debug representation of the rig's capsule:
	// (You'd need a debug mesh for the capsule and add it as a child or directly draw it here)
	// For now, it just calls the base class, which will draw any asset assigned to the rig.
	lsObject::Draw(cam, pass, scn, render);
}


// *** Inputs ***

void lsFirstPersonRig::HandleKeyboardInput(bool forward, bool backward, bool strafeLeft, bool strafeRight, bool crouch, bool jump, bool sprint)
{
	if (!m_bulletObject || !m_bulletObject->getBody()) return;

	// Update crouching state
	m_isCrouching = crouch;

	// Ground check before allowing movement
	checkGroundCollision();

	btVector3 linearVelocity = m_bulletObject->getBody()->getLinearVelocity();
	btVector3 currentForwardDir(ModelMatrix.matrix[8], ModelMatrix.matrix[9], ModelMatrix.matrix[10]); // -Z axis
	currentForwardDir.normalize();
	
	btVector3 currentRightDir(ModelMatrix.matrix[0], ModelMatrix.matrix[1], ModelMatrix.matrix[2]); // X axis
	currentRightDir.normalize();
	
	btVector3 movement = btVector3(0, 0, 0);
	float speed = m_walkSpeed;
	
	if (sprint) {
		speed = m_sprintSpeed;
	} else if (m_isCrouching) {
		speed = m_crouchSpeed;
	}
	
	// Movement handling
	if (forward)  movement -= currentForwardDir;
	if (backward) movement += currentForwardDir;
	if (strafeLeft)  movement -= currentRightDir;
	if (strafeRight) movement += currentRightDir;
	
	bool canMove = m_isOnGround;
	
	// Apply momentum during jumps
	if (!m_isOnGround && !jump)
	{
		// Apply stored movement momentum when in air
		movement = m_movement;
		
		// Maintain momentum
		linearVelocity.setX(movement.getX());
		linearVelocity.setZ(movement.getZ());
		// Air resistance
		linearVelocity.setX(linearVelocity.getX() * m_airResistance);
		linearVelocity.setZ(linearVelocity.getZ() * m_airResistance);
		
	} else if (canMove && movement.length2() > 0.0f)
	{
		// Normalize movement to prevent diagonal speed boost
		movement.normalize();
		movement *= speed;
		
		// Apply only X/Z components; preserve Y (gravity) from physics
		linearVelocity.setX(movement.getX());
		linearVelocity.setZ(movement.getZ());
		
		// Store current movement for momentum
		m_movement = movement;
	} else if (m_isOnGround)
	{
		// Ground friction: stop faster when on ground
		linearVelocity.setX(linearVelocity.getX() * 0.8f);
		linearVelocity.setZ(linearVelocity.getZ() * 0.8f);
		
		// Reset movement momentum when touching ground
		m_movement = btVector3(0, 0, 0);
	}
	
	// Jumping logic - only allow if on ground
	if (jump && m_isOnGround) {
		linearVelocity.setY(m_jumpForce); // Apply upward impulse
		m_isJumping = true;
		m_isOnGround = false; // Mark as not on ground immediately after jump
		// Store movement direction before jump for momentum
		m_movement = movement;
	} else if (!jump && m_isJumping && std::abs(linearVelocity.getY()) < 0.1f) {
		m_isJumping = false;
	}
	
	m_bulletObject->getBody()->setLinearVelocity(linearVelocity);
	
	// Update capsule shape and camera offset for crouching/standing
	float targetCapsuleCylinderHeight = m_isCrouching ?
		(m_crouchingCapsuleHeight - 2 * m_capsuleRadius) :
		(m_standingCapsuleHeight - 2 * m_capsuleRadius);

	float targetCameraOffset = m_isCrouching ?
		m_crouchingCameraOffset :
		m_standingCameraOffset;
	
	m_currentCapsuleHeight = targetCapsuleCylinderHeight;
	m_cameraHeightOffset = targetCameraOffset;
	updateCapsuleShape();
	
	m_bulletObject->getBody()->activate();
}

void lsFirstPersonRig::HandleMouseLook(float delta, float delta_x, float delta_y)
{
	// Apply yaw to the rig's rotation (around Y axis)
	// Horizontal mouse movement rotates the entire rig (and thus the camera with it)
	float yaw_delta = -delta_x * m_mouseXSensitivity * delta;
	
	// Get current physics body transform
	btTransform currentTransform = m_bulletObject->getBody()->getWorldTransform();
	
	// Extract current rotation quaternion from physics body
	btQuaternion currentRotation = currentTransform.getRotation();
	
	// Create yaw rotation quaternion (around Y axis)
	btQuaternion yawRotation;
	yawRotation.setEuler(yaw_delta, 0.0f, 0.0f);
	
	// Apply yaw rotation to current rotation
	currentRotation = yawRotation * currentRotation;
	currentRotation.normalize();
	
	// Update the physics body's rotation
	currentTransform.setRotation(currentRotation);
	m_bulletObject->getBody()->setWorldTransform(currentTransform);
	m_bulletObject->getBody()->getMotionState()->setWorldTransform(currentTransform);
	
	// Apply pitch to the camera's local rotation (around its X axis)
	// Vertical mouse movement rotates the camera up/down relative to the rig
	float pitch_delta = -delta_y * m_mouseYSensitivity * delta; // Assuming down mouse moves camera up
	m_camera->rotation.x += pitch_delta;
	// Clamp pitch to prevent looking too far up/down (e.g., -89 to +89 degrees)
	m_camera->rotation.x = std::clamp(m_camera->rotation.x, -89.0f, 89.0f);
	
	m_bulletObject->getBody()->activate(); // Activate body to ensure it responds to changes
}

void lsFirstPersonRig::checkGroundCollision()
{
	if (!m_bulletObject || !m_bulletObject->getBody()) return;
	
	btRigidBody* body = m_bulletObject->getBody();
	btVector3 position = body->getWorldTransform().getOrigin();
	
	// Determine the current capsule's half height
	btCollisionShape* shape = body->getCollisionShape();
	btCapsuleShape* capsuleShape = dynamic_cast<btCapsuleShape*>(shape);
	
	if (!capsuleShape) {
		// Fallback or error handling if shape isn't a capsule
		m_isOnGround = false;
		return;
	}
	
	// Get the total height including radius
	float halfHeight = capsuleShape->getHalfHeight();
	float radius = capsuleShape->getRadius();
	float totalHeight = halfHeight + radius;
	
	// Add a small offset to detect ground before contact
	float rayLength = totalHeight + 0.1f; // Margin for detection
	
	btVector3 rayStart = position;
	btVector3 rayEnd = position - btVector3(0.0f, rayLength, 0.0f);
	
	btCollisionWorld::ClosestRayResultCallback rayCallback(rayStart, rayEnd);
	
	lsScene* scene = Get_Scene();
	if (scene) {
		lsBulletWorld* bw = scene->getBulletworld();
		if (bw) {
			btDiscreteDynamicsWorld* world = bw->getDynamicsWorld();
			if (world) {
				world->rayTest(rayStart, rayEnd, rayCallback);

				// Set ground state
				m_isOnGround = rayCallback.hasHit();

				// Optional: Prevent false positives when moving upward quickly
				if (m_isOnGround && body->getLinearVelocity().getY() > 0.1f) {
					m_isOnGround = false;
				}
			}
		}
	}
}

void lsFirstPersonRig::updateCapsuleShape()
{
	if (!m_bulletObject || !m_bulletObject->getBody()) return;

	btCollisionShape* currentShapeRaw = m_bulletObject->getBody()->getCollisionShape();
	btCapsuleShape* currentShape = static_cast<btCapsuleShape*>(currentShapeRaw);
	btCapsuleShape* newShape = nullptr;

	// Determine target shape based on crouching state
	if (m_isCrouching && currentShape != m_crouchingShape) {
		newShape = m_crouchingShape;
	} else if (!m_isCrouching && currentShape != m_standingShape) {
		newShape = m_standingShape;
	}

	if (newShape && newShape != currentShape) {
		// Preserve current world transform
		btTransform currentTransform = m_bulletObject->getBody()->getWorldTransform();

		// Remove old body from world
		m_bulletObject->RemoveFromSim();

		// Update the collision shape
		m_bulletObject->getBody()->setCollisionShape(newShape);

		// Calculate old and new total half heights (radius + half cylinder height)
		float oldHalfHeight = currentShape->getHalfHeight() + currentShape->getRadius();
		float newHalfHeight = newShape->getHalfHeight() + newShape->getRadius();

		// Adjust position to keep the bottom of the capsule at the same Y-level
		// We move the center of mass up/down by the difference in half-heights
		btVector3 currentOrigin = currentTransform.getOrigin();
		float deltaHeight = newHalfHeight - oldHalfHeight;
		currentOrigin.setY(currentOrigin.getY() + deltaHeight);
		currentTransform.setOrigin(currentOrigin);
		m_bulletObject->getBody()->setWorldTransform(currentTransform);
		m_bulletObject->getBody()->getMotionState()->setWorldTransform(currentTransform);

		// Add back to world (this also copies transform to the new shape)
		m_bulletObject->AddToSim();

		// Reactivate body
		m_bulletObject->getBody()->activate();
	}
}

const std::string& lsFirstPersonRig::toString()
{
	save_str = "add_first_person_rig ";
	if (getName() != "") {
		save_str += "`";
		save_str += getName();
		save_str += "`";
	} else {
		save_str += "`FirstPersonRig`";
	}

	if (getParent()) {
		save_str += " `";
		save_str += getParent()->getName();
		save_str += "`";
	} else {
		save_str += " `NONE`";
	}

	// Pos, Rot and scale
	save_str += " ";
	save_str += position.toString();
	save_str += " ";
	save_str += rotation.toString();
	save_str += " ";
	save_str += scale.toString();
	
	// Add specific FPS rig parameters if needed for saving/loading state
	// Example: m_walkSpeed, m_crouchSpeed, m_jumpForce, m_mouseSensitivity, etc.
	// For brevity, not including all of them here, but a real implementation would.

	return save_str;
}


