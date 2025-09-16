#include "lsNode_FirstPersonRig.h"

#include "lsScene.h"
#include "lsCamera.h"
#include "../physics/lsBulletObject.h"
#include "../physics/lsBulletWorld.h"
#include "../assets/lsAssetManager.h"
#include "../assets/lsAsset_BulletShape_Pill.h"
#include "../datablocks/lsBulletShape.h"
#include "../math/lsMath.h"
#include <cmath> // For sin, cos

lsNode_FirstPersonRig::lsNode_FirstPersonRig(lsNode* parent) 
    : lsObject(parent), m_wasCrouching(false)
{
    setNode_type(LSNT_FIRSTPERSONRIG);
    setName("FirstPersonRig");

    lsScene* scene = Get_Scene();
    if (!scene) {
        std::cerr << "ERROR: lsNode_FirstPersonRig must be added to a scene." << std::endl;
        return;
    }
	
    // 1. Create the physics shape assets for standing and crouching
    m_standShapeAsset = scene->getAssetManager()->CreateAsset_BulletShape_Pill(m_capsuleRadius, m_standingCapsuleHeight);
    m_standShapeAsset->AllocAsset();
    m_crouchShapeAsset = scene->getAssetManager()->CreateAsset_BulletShape_Pill(m_capsuleRadius, m_crouchingCapsuleHeight);
    m_crouchShapeAsset->AllocAsset();

    // 2. Create the bullet object with the standing shape
    lsBulletShape* bulletShape = static_cast<lsBulletShape*>(m_standShapeAsset->getData());
    m_bulletObject = new lsBulletObject(this, bulletShape->getShape(), scene->getBulletworld()->getMax_proxies());
    m_bulletObject->Init(80.0f); // Mass of 80kg
	
    // 3. Configure the rigid body
    btRigidBody* body = m_bulletObject->getBody();
    if(body)
    {
        body->setAngularFactor(btVector3(0, 1, 0)); // Only allow rotation around Y (yaw)
        body->setSleepingThresholds(0.0, 0.0);      // Prevent player from sleeping
        body->setFriction(0.7f);
        body->setDamping(0.9f, 0.0f); // High linear damping for control, no angular damping
        body->setActivationState(DISABLE_DEACTIVATION);
    }

    // 4. Create and attach the camera
    m_camera = new lsCamera(this);
    m_camera->setName("RigCamera");
    m_camera->Set_Pos(0, m_standingCamHeight, 0); // Initial camera height
    Add_Node(m_camera);
}

lsNode_FirstPersonRig::~lsNode_FirstPersonRig()
{
    // lsBulletObject is deleted by lsObject's destructor
    // m_standShapeAsset and m_crouchShapeAsset are managed by the AssetManager
}


void lsNode_FirstPersonRig::handleInput(bool forward, bool backward, bool left, bool right, bool jump, bool sprint, bool crouch)
{
    m_moveForward = forward;
    m_moveBackward = backward;
    m_strafeLeft = left;
    m_strafeRight = right;
    m_wantsToJump = jump;
    m_isSprinting = sprint;
    m_isCrouching = crouch;
}

void lsNode_FirstPersonRig::handleMouseLook(float deltaYaw, float deltaPitch)
{
    m_yaw -= deltaYaw * m_mouseSensitivity;
    m_pitch -= deltaPitch * m_mouseSensitivity;
	
    // Clamp pitch to avoid flipping over
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
}

void lsNode_FirstPersonRig::updateCapsuleShape()
{
    if (!m_bulletObject || !m_bulletObject->getBody()) return;

    btRigidBody* body = m_bulletObject->getBody();
    btCollisionShape* newShape = nullptr;

    if (m_isCrouching) {
        newShape = static_cast<lsBulletShape*>(m_crouchShapeAsset->getData())->getShape();
    } else {
        newShape = static_cast<lsBulletShape*>(m_standShapeAsset->getData())->getShape();
    }

    if (body->getCollisionShape() != newShape) {
        body->setCollisionShape(newShape);
        
        // When changing shape, the physics world needs to be updated about the object's AABB
        lsScene* scene = Get_Scene();
        if (scene && scene->getBulletworld() && scene->getBulletworld()->getDynamicsWorld()) {
            btDynamicsWorld* world = scene->getBulletworld()->getDynamicsWorld();
            world->removeRigidBody(body);
            world->addRigidBody(body);
        }
    }
}

bool lsNode_FirstPersonRig::onGround()
{
    if (!m_bulletObject || !m_bulletObject->getBody()) return false;
	
    btVector3 start = m_bulletObject->getBody()->getWorldTransform().getOrigin();
    float currentCapsuleHeight = m_isCrouching ? m_crouchingCapsuleHeight : m_standingCapsuleHeight;
    // Ray starts from the center and goes down just past the bottom of the capsule
    btVector3 end = start - btVector3(0, currentCapsuleHeight * 0.5f + m_capsuleRadius + 0.1f, 0);
	
    btCollisionWorld::ClosestRayResultCallback rayCallback(start, end);
    Get_Scene()->getBulletworld()->getDynamicsWorld()->rayTest(start, end, rayCallback);
	
    return rayCallback.hasHit();
}

void lsNode_FirstPersonRig::Move(lsScene* scn, lsOpenGL* opengl, double delta)
{
    if (!m_bulletObject || !m_bulletObject->getBody()) {
        lsObject::Move(scn, opengl, delta); // Fallback to base behavior if no physics
        return;
    }

    // Check for crouch state change and update physics shape
    if (m_isCrouching != m_wasCrouching) {
        updateCapsuleShape();
        m_wasCrouching = m_isCrouching;
    }

    btRigidBody* body = m_bulletObject->getBody();

    // 1. Update Rotation from Mouse Input
    // We apply yaw rotation directly to the rigid body's transform.
    btTransform trans = body->getWorldTransform();
    btQuaternion yawQuat(btVector3(0, 1, 0), m_yaw * ANG2RAD);
    trans.setRotation(yawQuat);
    body->setWorldTransform(trans);

    // 2. Update Velocity from Keyboard Input
    // We set linear velocity directly for responsive, arcade-like movement.
    btVector3 currentVelocity = body->getLinearVelocity();
    btVector3 desiredVelocity(0, currentVelocity.y(), 0); // Preserve Y velocity for gravity/jumping

    // Calculate forward and side vectors based on the current yaw.
    btVector3 forwardDir(-sin(m_yaw * ANG2RAD), 0, -cos(m_yaw * ANG2RAD));
    btVector3 sideDir(-cos(m_yaw * ANG2RAD), 0, sin(m_yaw * ANG2RAD)); // Left vector

    btVector3 moveDirection(0, 0, 0);
    if (m_moveForward)  moveDirection += forwardDir;
    if (m_moveBackward) moveDirection -= forwardDir;
    if (m_strafeLeft)   moveDirection += sideDir;
    if (m_strafeRight)  moveDirection -= sideDir;

    float currentSpeed = m_isCrouching ? m_crouchSpeed : (m_isSprinting ? m_sprintSpeed : m_walkSpeed);
    bool is_on_ground = onGround();

    if (moveDirection.length2() > 0.0f) {
        moveDirection.normalize();
        desiredVelocity.setX(moveDirection.x() * currentSpeed);
        desiredVelocity.setZ(moveDirection.z() * currentSpeed);
    } else {
        // If there's no input, we want to stop horizontal movement.
        // The high linear damping on the rigid body will handle this smoothly.
        desiredVelocity.setX(0);
        desiredVelocity.setZ(0);
    }

    if (m_wantsToJump && is_on_ground && !m_isCrouching) {
        desiredVelocity.setY(m_jumpSpeed); // Apply jump velocity
    }

    body->setLinearVelocity(desiredVelocity);
    body->activate(true); // Ensure the body doesn't go to sleep

    // --- 3. Update Camera ---
    // Adjust camera height for crouching
    float targetCamHeight = m_isCrouching ? m_crouchedCamHeight : m_standingCamHeight;
    // Simple lerp for smooth transition
    m_camera->position.y = m_camera->position.y * 0.9f + targetCamHeight * 0.1f;
    m_camera->Set_Rot(m_pitch, 0, 0);

    // --- 4. Update Node Transform ---
    // Let the base lsObject::Move handle applying the physics transform back to this node's ModelMatrix
    lsObject::Move(scn, opengl, delta);
}

void lsNode_FirstPersonRig::Optimize(lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl)
{
    // This node is not directly rendered, so it doesn't need to be added to the draw list.
    // We still pass the call down to its children (like the camera).
    if (getNode_list()) {
        getNode_list()->Optimize(cam, pass, scn, opengl);
    }
}

const string& lsNode_FirstPersonRig::toString()
{
    save_str = "add_firstpersonrig";
    return save_str;
}
