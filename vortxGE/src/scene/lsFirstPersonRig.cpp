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
#include <algorithm> // For clamp
#include <cmath>     // For lerp


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
	m_mouseYSensitivity = 1.85f;
	m_airResistance = 0.95;		// Slows down the player if not on the ground.
	
	// Pickup objects settings.
	m_pickupDistance = 5.0f;     // Distance to pickup objects
	m_throwForce = 30.0f;        // Force applied when throwing (function also calc's mass)
	m_holdDistance = 4.0f;       // Distance in front of player where object is held
	m_isHolding = false;
	
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
	m_targetCameraHeightOffset = m_cameraHeightOffset;
	m_cameraLerpSpeed = 0.1f; // 10% per frame for smooth interpolation
	
	// Create bullet shapes using the scene's asset manager
	lsAssetManager* am = scene->getAssetManager();
	if (!am)
	{
		cerr << "ERROR: lsFirstPersonRig - AssetManager not available from scene." << endl;
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
		cerr << "ERROR: lsFirstPersonRig - Failed to create standing capsule asset." << endl;
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
		cerr << "ERROR: lsFirstPersonRig - Failed to create crouching capsule asset." << endl;
	}
	
	// setting up physics
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
			// Set angular high to stop held objects from rotation the player to much.
			body->setDamping(0.9f, 2.0f); // Linear and angular damping
			
			// Set friction and restitution for sliding behavior
			body->setFriction(0.1f); // Low friction for sliding
			body->setRollingFriction(0.1f);
			body->setSpinningFriction(0.1f);
			body->setRestitution(0.0f); // No bounce
			
			body->setGravity(dynamicsWorld->getGravity()); // Inherit world gravity
			
			setBulletObject(m_bulletObject);
		} else
		{
			cerr << "ERROR: lsFirstPersonRig - Failed to create BulletObject." << endl;
		}
	} else
	{
		cerr << "ERROR: lsFirstPersonRig - Standing capsule shape is NULL, cannot initialize BulletObject." << endl;
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
		cerr << "ERROR: lsFirstPersonRig - Failed to create camera." << endl;
	}
}

lsFirstPersonRig::~lsFirstPersonRig()
{
	// Clean up any held objects before destruction
	if (m_constraint)
	{
		cleanupConstraint();
	}
	
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
	
	// Check if any held object is marked for deletion and clean it up
    if (m_isHolding && m_heldObject)
    {
        if (m_heldObject->IsMarkedForDeletion())
        {
            releaseObject();
        }
    }
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

	// Update target camera height for interpolation
	m_targetCameraHeightOffset = crouch ? m_crouchingCameraOffset : m_standingCameraOffset;
	
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
	} else if (!jump && m_isJumping && abs(linearVelocity.getY()) < 0.1f) {
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
	m_cameraHeightOffset = lerp(m_cameraHeightOffset, targetCameraOffset, m_cameraLerpSpeed);
	m_targetCameraHeightOffset = targetCameraOffset;
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
	m_camera->rotation.x = clamp(m_camera->rotation.x, -89.0f, 89.0f);
	
	m_bulletObject->getBody()->activate(); // Activate body to ensure it responds to changes
}

void lsFirstPersonRig::HandleMouseButtons(bool left, bool right)
{
	// Left mouse button: only trigger throwing if holding an object
	if (left)
	{
		if (m_isHolding)
		{
			throwObject();
			return;
		}
	}
	
	// Right mouse button: pickup or release based on state
	if (right && m_isHolding) return;
	if (right && !m_isHolding)
	{
		tryPickupObject();
		return;
	}
	// Just let go
	releaseObject();
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

void lsFirstPersonRig::tryPickupObject()
{
	if (m_isHolding || !m_bulletObject) return;
	
	// Raycast to find objects in front of the player
	btVector3 rayStart = btVector3(
		m_camera->ModelMatrix.matrix[12],
		m_camera->ModelMatrix.matrix[13],
		m_camera->ModelMatrix.matrix[14]
	);
	// Ray direction
	btVector3 rayDirection = btVector3(
		m_camera->ModelMatrix.matrix[8],
		m_camera->ModelMatrix.matrix[9],
		m_camera->ModelMatrix.matrix[10]
	);
	rayDirection = rayDirection.normalize();
	btVector3 rayEnd = rayStart + -(rayDirection * m_pickupDistance);
	
	// Create a ray test callback
	btCollisionWorld::ClosestRayResultCallback rayCallback(rayStart, rayEnd);
	
	lsScene* scene = Get_Scene();
	if (scene) {
		lsBulletWorld* bw = scene->getBulletworld();
		if (bw) {
			btDiscreteDynamicsWorld* world = bw->getDynamicsWorld();
			if (world) {
				world->rayTest(rayStart, rayEnd, rayCallback);
				
				if (rayCallback.hasHit()) {
					const btCollisionObject* hitObject = rayCallback.m_collisionObject;
					lsObject* obj = static_cast<lsObject*>(hitObject->getUserPointer());
					
					if (obj && obj != this) { // Don't pick up self
						pickupObject(obj);
					}
				}
			}
		}
	}
}

void lsFirstPersonRig::pickupObject(lsObject* obj)
{
	if (!obj || !m_bulletObject || m_isHolding) return;
	
	btRigidBody* body = static_cast<btRigidBody*>(obj->getBulletObject()->getBody());
	if (!body) return;
	
	// Skip objects with zero or negative mass (static/kinematic bodies)
	if (body->getMass() <= 0.0f) return;
	
	m_heldObject = obj;
	m_heldBody = body;
	
	// Disable sleeping for the held object to keep it active
	body->setSleepingThresholds(0.0f, 0.0f);
	
	// Get player's position and orientation
	btVector3 playerPosition(0.0, 1.0, 0.0);
	btVector3 forwardDir(0.0, 0.0, -1.0);
	// Calculate desired offset in front of player (relative to player's local space)
	btVector3 desiredOffset = playerPosition + forwardDir * m_holdDistance;
	
	// Create constraint frames
	btTransform constraintFrameA = btTransform::getIdentity();
	btTransform constraintFrameB = btTransform::getIdentity();

	// Set constraint frame B to be at the center of the object (local space)
	constraintFrameB.setOrigin(btVector3(0, 0, 0));
	
	// Set constraint frame A such that it positions the object at 'desiredOffset' from player
	constraintFrameA.setOrigin(desiredOffset);

	// Create 6DOF constraint between player and object
	m_constraint = new btGeneric6DofConstraint(
		*m_bulletObject->getBody(), 
		*body,
		constraintFrameA,
		constraintFrameB.inverse(),
		true
	);

	// Allow only small movement (to keep object near player)
	m_constraint->setLinearLowerLimit(btVector3(-0.2f, -0.2f, -0.2f));
	m_constraint->setLinearUpperLimit(btVector3(0.2f, 0.2f, 0.2f));

	// Prevent rotation of the held object
	m_constraint->setAngularLowerLimit(btVector3(0, 0, 0));
	m_constraint->setAngularUpperLimit(btVector3(0, 0, 0));

	// Apply constraint with proper world coordinates
	lsScene* scene = Get_Scene();
	if (scene)
	{
		lsBulletWorld* bw = scene->getBulletworld();
		if (bw)
		{
			btDiscreteDynamicsWorld* world = bw->getDynamicsWorld();
			if (world)
			{
				world->addConstraint(m_constraint, true); // Enable collisions between constrained objects
				m_isHolding = true;
			}
		}
	}
}

void lsFirstPersonRig::releaseObject()
{
	if (!m_isHolding || !m_constraint) return;
	
	// Remove constraint from world
	lsScene* scene = Get_Scene();
	if (scene)
	{
		lsBulletWorld* bw = scene->getBulletworld();
		if (bw)
		{
			btDiscreteDynamicsWorld* world = bw->getDynamicsWorld();
			if (world)
			{
				world->removeConstraint(m_constraint);
			}
		}
	}
	
	// Clean up constraint
	cleanupConstraint();
	
	// Restore the held object's original physics properties including sleep state
	if (m_heldBody)
	{
		// Restore sleep thresholds to allow sleeping again
		m_heldBody->setSleepingThresholds(0.8f, 1.0f);
	}
	
	// Reset held object state
	m_heldObject = nullptr;
	m_heldBody = nullptr;
	m_isHolding = false;
}

void lsFirstPersonRig::throwObject()
{
	if (!m_isHolding || !m_heldBody) return;
	
	// Apply force to the held object
	btVector3 throwDirection = btVector3(
		m_camera->ModelMatrix.matrix[8],
		m_camera->ModelMatrix.matrix[9],
		m_camera->ModelMatrix.matrix[10]
	);
	throwDirection = throwDirection.normalize();
	
	// Calculate the required impulse based on mass and desired speed
	btVector3 force = -throwDirection * (m_throwForce * m_heldBody->getMass());
	m_heldBody->applyImpulse(force, btVector3(0, 0, 0));
	
	// Release the object
	releaseObject();
}

void lsFirstPersonRig::cleanupConstraint()
{
	if (m_constraint)
	{
		lsScene* scene = Get_Scene();
		if (scene)
		{
			lsBulletWorld* bw = scene->getBulletworld();
			if (bw)
			{
				btDiscreteDynamicsWorld* world = bw->getDynamicsWorld();
				if (world)
				{
					world->removeConstraint(m_constraint);
				}
			}
		}
		delete m_constraint;
		m_constraint = nullptr;
	}
}

// If a held object is marked for deletion we need to release it.
// Clean up before deletion
void lsFirstPersonRig::CleanupBeforeDeletion()
{
	// Release any held object if needed
	if (m_isHolding)
	{
		releaseObject();
	}
}


const string& lsFirstPersonRig::toString()
{
	save_str = "add_first_person_rig ";
	if (getName() != "")
	{
		save_str += "`";
		save_str += getName();
		save_str += "`";
	} else
	{
		save_str += "`FirstPersonRig`";
	}
	
	if (getParent())
	{
		save_str += " `";
		save_str += getParent()->getName();
		save_str += "`";
	} else
	{
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

