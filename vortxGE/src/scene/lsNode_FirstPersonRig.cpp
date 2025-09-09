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

lsNode_FirstPersonRig::lsNode_FirstPersonRig(lsNode* parent) : lsObject(parent)
{
    setNode_type(LSNT_FIRSTPERSONRIG);
    setName("FirstPersonRig");

    lsScene* scene = Get_Scene();
    if (!scene) {
        std::cerr << "ERROR: lsNode_FirstPersonRig must be added to a scene." << std::endl;
        return;
    }

    // 1. Create the physics shape asset (Radius 0.5, Height 1.0 -> Total height 2.0)
    m_shapeAsset = scene->getAssetManager()->CreateAsset_BulletShape_Pill(0.5f, m_capsuleHeight);
    m_shapeAsset->AllocAsset();

    // 2. Create the bullet object
    lsBulletShape* bulletShape = static_cast<lsBulletShape*>(m_shapeAsset->getData());
    m_bulletObject = new lsBulletObject(this, bulletShape->getShape(), scene->getBulletworld()->getMax_proxies());
    m_bulletObject->Init(80.0f); // Mass of 80kg

    // 3. Configure the rigid body
    btRigidBody* body = m_bulletObject->getBody();
    if (body) {
        body->setAngularFactor(btVector3(0, 1, 0)); // Only allow rotation around Y (yaw)
        body->setSleepingThresholds(0.0, 0.0);      // Prevent player from sleeping
        body->setFriction(0.7f);
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
    // m_shapeAsset is managed by the AssetManager
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
    m_yaw -= deltaYaw * m_mouseSensitivity; // Invert for standard controls
    m_pitch += deltaPitch * m_mouseSensitivity; // Invert for standard controls

    // Clamp pitch to avoid flipping
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
}

bool lsNode_FirstPersonRig::onGround()
{
    if (!m_bulletObject || !m_bulletObject->getBody()) return false;
	
    btVector3 start = m_bulletObject->getBody()->getWorldTransform().getOrigin();
    // Ray starts from the center and goes down just past the bottom of the capsule
    btVector3 end = start - btVector3(0, m_capsuleHeight * 0.5f + 0.6f, 0);
	
    btCollisionWorld::ClosestRayResultCallback rayCallback(start, end);
    Get_Scene()->getBulletworld()->getDynamicsWorld()->rayTest(start, end, rayCallback);
	
    return rayCallback.hasHit();
}

void lsNode_FirstPersonRig::Move(lsScene* scn, lsOpenGL* /* opengl */, double delta)
{
    if (!m_bulletObject || !m_bulletObject->getBody()) return;

    btRigidBody* body = m_bulletObject->getBody();

    // 1. Apply Yaw rotation to the physics body from mouse input
    btTransform trans;
    body->getMotionState()->getWorldTransform(trans);
    btQuaternion yawQuat;
    yawQuat.setEuler(m_yaw * ANG2RAD, 0, 0); // Bullet uses Y-up, setEuler is YXZ
    trans.setRotation(yawQuat);
    body->getMotionState()->setWorldTransform(trans);
    body->setCenterOfMassTransform(trans);
	
    // 2. Handle movement and jumping, only if on the ground
    bool is_on_ground = onGround();

    if (is_on_ground)
    {
        // Calculate target velocity from keyboard input
        float speed = m_isCrouching ? m_crouchSpeed : (m_isSprinting ? m_sprintSpeed : m_walkSpeed);
        btVector3 forwardDir(sin(m_yaw * ANG2RAD), 0, cos(m_yaw * ANG2RAD));
        btVector3 sideDir(-cos(m_yaw * ANG2RAD), 0, sin(m_yaw * ANG2RAD));

        btVector3 targetVelocity(0, 0, 0);
        if (m_moveForward)  targetVelocity += forwardDir;
        if (m_moveBackward) targetVelocity -= forwardDir;
        if (m_strafeLeft)   targetVelocity -= sideDir;
        if (m_strafeRight)  targetVelocity += sideDir;

        if (targetVelocity.length2() > 0) {
            targetVelocity.normalize();
            targetVelocity *= speed;
        }

        // Calculate the force needed to reach the target velocity in one timestep
        btVector3 currentVelocity = body->getLinearVelocity();
        // We only want to affect horizontal velocity
        btVector3 velocityChange = targetVelocity - btVector3(currentVelocity.x(), 0, currentVelocity.z());

        // Convert delta from ms to seconds for physics calculation
        float dt = delta / 1000.0f;
        if (dt > 0) {
            // F = m * a = m * (dv/dt)
            btVector3 requiredForce = (velocityChange / dt) * body->getMass();
            body->applyCentralForce(requiredForce);
        }

        // Handle Jumping
        if (m_wantsToJump)
        {
            // Apply a vertical impulse for the jump
            body->applyCentralImpulse(btVector3(0, m_jumpForce, 0));
        }
    }
    
    // 3. Handle Crouching (camera height)
    float targetCamHeight = m_isCrouching ? m_crouchedCamHeight : m_standingCamHeight;
    m_camera->position.y = targetCamHeight;


    // 4. Update this node's transform from the physics simulation
    m_bulletObject->Apply_Physics(this);

    // 5. Update the child camera's local rotation for pitch (mouse look up/down)
    m_camera->Set_Rot(m_pitch, 0, 0);

    // Life
    checkLife(scn);
}

void lsNode_FirstPersonRig::Optimize(lsCamera* cam, lsPass* pass, lsScene* scn, lsOpenGL* opengl)
{
    // This node is not directly rendered, so it doesn't need to be added to the draw list.
    // We still pass the call down to children (like the camera).
    if (getNode_list()) {
        getNode_list()->Optimize(cam, pass, scn, opengl);
    }
}

const string& lsNode_FirstPersonRig::toString()
{
    save_str = "add_firstpersonrig";
    return save_str;
}
