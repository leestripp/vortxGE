/**
 * @file lsFirstPersonRig.h
 * @brief First-person character controller with physics
 * 
 * Provides a first-person player controller with physics integration,
 * including movement, jumping, crouching, and object pickup mechanics.
 * 
 * @author vortxGE
 * @date 2024
 * @see lsObject
 * @see lsCamera
 * @see lsBulletObject
 */

#pragma once

#include "lsObject.h" 
#include "lsCamera.h"
#include "../physics/lsBulletObject.h"
#include "../physics/lsBulletWorld.h"
#include <bullet/btBulletDynamicsCommon.h>

// Forward declarations
class lsScene;
class lsRender;
class lsAssetManager;
class lsBulletShape;

/**
 * @class lsFirstPersonRig
 * @brief First-person character controller
 * 
 * A first-person player controller that combines physics simulation with
 * camera control. Supports:
 * - Walking and sprinting
 * - Jumping with ground detection
 * - Crouching with camera height interpolation
 * - Object pickup and throw mechanics
 * - Physics-based capsule collision
 * 
 * The rig uses a capsule collider for physics and maintains a child camera
 * for rendering. Movement is applied via physics forces.
 * 
 * @note Inherits from lsObject which inherits from lsAnimNode
 * 
 * @example
 * @code
 * lsScene* scene;
 * btDiscreteDynamicsWorld* dynamicsWorld;
 * 
 * lsFirstPersonRig* player = new lsFirstPersonRig(scene, dynamicsWorld);
 * player->Set_Pos(lsVector3(0, 5, 0));
 * 
 * // In update loop
 * player->HandleKeyboardInput(forward, backward, left, right, crouch, jump, sprint);
 * player->HandleMouseLook(deltaTime, mouseDX, mouseDY);
 * player->Move(scene, renderer, deltaTime);
 * @endcode
 */
class lsFirstPersonRig : public lsObject
{
public:
	/**
	 * @brief Construct first-person rig
	 * 
	 * @param scene Pointer to the scene
	 * @param dynamicsWorld Pointer to Bullet physics world
	 */
	lsFirstPersonRig(lsScene* scene, btDiscreteDynamicsWorld* dynamicsWorld);
	
	/// Destructor
	virtual ~lsFirstPersonRig();
	
	//=========================================================================
	// lsObject Virtual Overrides
	//=========================================================================
	
	/**
	 * @brief Update player state each frame
	 * 
	 * Called each frame to update physics and movement.
	 * 
	 * @param scn Pointer to scene
	 * @param render Pointer to renderer
	 * @param delta Time since last frame in seconds
	 */
	virtual void Move(lsScene* scn, lsRender* render, double delta) override;
	
	/**
	 * @brief Optimize player for rendering
	 * 
	 * @param cam Pointer to camera
	 * @param pass Pointer to render pass
	 * @param scn Pointer to scene
	 * @param render Pointer to renderer
	 */
	virtual void Optimize(lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render) override;
	
	/**
	 * @brief Draw the player (typically hidden in FPS)
	 * 
	 * @param cam Pointer to camera
	 * @param pass Pointer to render pass
	 * @param scn Pointer to scene
	 * @param render Pointer to renderer
	 */
	virtual void Draw(lsCamera* cam, lsPass* pass, lsScene* scn, lsRender* render) override;
	
	//=========================================================================
	// Input Handling
	//=========================================================================
	
	/**
	 * @brief Handle keyboard movement input
	 * 
	 * Processes WASD-style movement plus jump, crouch, and sprint.
	 * 
	 * @param forward Move forward
	 * @param backward Move backward
	 * @param strafeLeft Strafe left
	 * @param strafeRight Strafe right
	 * @param crouch Toggle crouch
	 * @param jump Jump (if on ground)
	 * @param sprint Sprint (faster movement)
	 */
	void HandleKeyboardInput(bool forward, bool backward, bool strafeLeft, bool strafeRight, bool crouch, bool jump, bool sprint);
	
	/**
	 * @brief Handle mouse look (rotation)
	 * 
	 * Rotates the camera based on mouse movement.
	 * 
	 * @param delta Time since last frame
	 * @param delta_x Mouse X delta
	 * @param delta_y Mouse Y delta
	 */
	void HandleMouseLook(float delta, float delta_x, float delta_y);
	
	/**
	 * @brief Handle mouse button states
	 * 
	 * @param left Left mouse button
	 * @param right Right mouse button
	 */
	void HandleMouseButtons(bool left, bool right);
	
	//=========================================================================
	// Pickup System
	//=========================================================================
	
	/**
	 * @brief Try to pick up nearby object
	 * 
	 * Attempts to find and pick up an object within pickup range.
	 * Uses ray casting to detect valid objects.
	 */
	void tryPickupObject();
	
	/**
	 * @brief Pick up a specific object
	 * 
	 * Attaches the object to the player using a physics constraint.
	 * 
	 * @param obj Object to pick up
	 */
	void pickupObject(lsObject* obj);
	
	/**
	 * @brief Release held object
	 * 
	 * Drops the currently held object without throwing.
	 */
	void releaseObject();
	
	/**
	 * @brief Throw held object
	 * 
	 * Releases the object and applies forward force.
	 */
	void throwObject();
	
	/**
	 * @brief Check if holding an object
	 * @return true if an object is currently held
	 */
	bool isHolding() const { return m_isHolding; }
	
	/**
	 * @brief Clean up before deletion
	 * 
	 * Called when the held object is being deleted to
	 * safely release the physics constraint.
	 */
	void CleanupBeforeDeletion();
	
	//=========================================================================
	// Accessors
	//=========================================================================
	
	/**
	 * @brief Get the first-person camera
	 * @return Pointer to the camera
	 */
	lsCamera* getCamera() const { return m_camera; }
	
	//=========================================================================
	// Serialization
	//=========================================================================
	
	/**
	 * @brief Serialize rig to string
	 * @return String representation
	 */
	virtual const string& toString() override;
	
private:
	//=========================================================================
	// Private Methods
	//=========================================================================
	
	/**
	 * @brief Check if player is on ground
	 * 
	 * Uses ray casting downward to detect ground contact.
	 */
	void checkGroundCollision();
	
	/**
	 * @brief Update physics capsule shape
	 * 
	 * Switches between standing and crouching collision shapes.
	 */
	void updateCapsuleShape();
	
	/**
	 * @brief Clean up physics constraint
	 * 
	 * Removes the pickup constraint from physics world.
	 */
	void cleanupConstraint();
	
	//=========================================================================
	// Private Members
	//=========================================================================
	
	/// First-person camera (child of this rig)
	lsCamera* m_camera;
	
	/// Physics rigid body for the player
	lsBulletObject* m_bulletObject;
	
	//=========================================================================
	// Physics Shapes
	//=========================================================================
	
	/// Capsule shape for standing
	btCapsuleShape* m_standingShape;
	
	/// Capsule shape for crouching
	btCapsuleShape* m_crouchingShape;
	
	/// Asset for standing shape (managed by scene)
	lsAsset* m_standingShapeAsset;
	
	/// Asset for crouching shape (managed by scene)
	lsAsset* m_crouchingShapeAsset;
	
	//=========================================================================
	// Movement Parameters
	//=========================================================================
	
	float m_walkSpeed;      	///< Normal walk speed
	float m_sprintSpeed;    	///< Sprint speed multiplier
	float m_crouchSpeed;    	///< Crouch speed
	float m_jumpForce;      	///< Jump impulse force
	float m_mouseXSensitivity;	///< Mouse X rotation sensitivity
	float m_mouseYSensitivity;	///< Mouse Y rotation sensitivity
	
	//=========================================================================
	// Ground Detection
	//=========================================================================
	
	bool m_isOnGround;				///< Is player on ground
	float m_groundCheckDistance;	///< Distance to check for ground
	btVector3 m_movement;			///< Current movement vector
	float m_airResistance;			///< Air resistance when jumping
	
	//=========================================================================
	// State
	//=========================================================================
	
	bool m_isCrouching;			///< Is crouching
	bool m_isJumping;				///< Is jumping
	float m_currentCapsuleHeight;	///< Current interpolated height
	float m_standingCapsuleHeight;	///< Standing capsule height
	float m_crouchingCapsuleHeight;	///< Crouching capsule height
	float m_capsuleRadius;			///< Capsule radius (same for both)
	
	//=========================================================================
	// Camera
	//=========================================================================
	
	float m_cameraHeightOffset;				///< Current camera Y offset
	float m_targetCameraHeightOffset;		///< Target camera Y offset
	float m_cameraLerpSpeed;				///< Camera lerp speed
	float m_standingCameraOffset;			///< Standing camera offset
	float m_crouchingCameraOffset;			///< Crouching camera offset
	
	//=========================================================================
	// Pickup System
	//=========================================================================
	
	bool m_isHolding;				///< Is holding object
	lsObject* m_heldObject;		///< Currently held object
	btRigidBody* m_heldBody;		///< Physics body of held object
	btGeneric6DofConstraint* m_constraint;	///< Physics constraint
	btTransform m_originalTransform;		///< Original transform
	
	float m_pickupDistance;		///< Max pickup distance
	float m_throwForce;			///< Force when throwing
	float m_holdDistance;			///< Distance to hold object
};
