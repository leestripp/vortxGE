/**
 * @file lsFlyCam.h
 * @brief Free-flying survey camera: animated node + child camera
 *
 * Architecture (canonical fly camera, see LearnOpenGL "Camera"):
 *  - lsFlyCam is an lsAnimNode carrying POSITION and YAW only
 *  - a child lsCamera carries PITCH (rotation.x) and is the scene's
 *    active camera. Because lsCamera::Move composes the parent's
 *    ModelMatrix, the combined orientation is Ry(yaw)*Rx(pitch) —
 *    the correct FPS/fly order the engine's single-node Euler
 *    rebuild (Rx*Ry*Rz) cannot express.
 *  - scripted flyovers (orbit, waypoint tours) are KEYFRAME
 *    ANIMATIONS on this node's Position/Rotation properties via the
 *    lsAnimationManager; manual input cancels playback.
 *
 * The demo wires it up with env hooks (VORTX_FLYCAM, VORTX_FLYPATH, ...)
 * so a whole map can be surveyed in one headless run.
 */
#ifndef LSFLYCAM_H
#define LSFLYCAM_H

#include "lsAnimNode.h"
#include "lsCamera.h"
#include "../math/lsVector3.h"

#include <vector>

class lsFlyCam : public lsAnimNode
{
public:
	lsFlyCam( lsNode* parent );
	virtual ~lsFlyCam();

	/// Child camera — set this as the scene's active camera.
	lsCamera* getCamera() const { return m_camera; }

	//=========================================================================
	// Movement / look (input-driven, consumed by Move)
	//=========================================================================

	/// Flight speed in metres/second (sprint multiplies x4).
	void setFlySpeed( float mps ) { m_flySpeed = mps; }
	float getFlySpeed() const { return m_flySpeed; }

	/**
	 * @brief Queue one frame of keyboard flight input.
	 *
	 * Stored as flags; Move() integrates them with the frame delta, so
	 * movement speed is frame-rate independent. Manual movement cancels
	 * any running path animation.
	 */
	void HandleFlyInput( bool fwd, bool back, bool left, bool right,
						 bool up, bool down, bool sprint );

	/// Mouse-look deltas (pixels). Positive dx turns right.
	void HandleFlyLook( float dx, float dy );

	/// Absolute orientation (degrees): yaw about world Y, pitch clamp ±89.
	void setYawPitch( float yawDeg, float pitchDeg );

	//=========================================================================
	// Scripted flyovers — keyframe animations on this node
	//=========================================================================

	/**
	 * @brief Circular orbit animation around centre (looped).
	 * @param lookAtCentre true: face the centre; false: face travel direction
	 */
	void Start_Orbit( lsVector3 centre, float radius, float periodSec,
					  float pitchDeg, bool lookAtCentre );

	/// Queue a waypoint (world position) for the tour.
	void Add_Waypoint( lsVector3 pos );

	/// Start the waypoint tour animation (looped, legs at flySpeed).
	void Start_Waypoints( float pitchDeg );

	/// Stop path playback; manual flight resumes from the current pose.
	void Stop_Path();

	bool isPathActive() const { return m_pathActive; }

	//=========================================================================
	// Update
	//=========================================================================

	virtual void Move( lsScene* scn, lsRender* render, double delta ) override;

private:
	/// Push m_yawDeg/m_pitchDeg to node yaw + child-camera pitch.
	void applyYawPitch();

	lsCamera* m_camera = nullptr;

	float m_flySpeed = 40.0f;   ///< m/s
	float m_yawDeg = 0.0f;      ///< node rotation.y
	float m_pitchDeg = 0.0f;    ///< child camera rotation.x

	// input state
	bool m_inFwd = false, m_inBack = false;
	bool m_inLeft = false, m_inRight = false;
	bool m_inUp = false, m_inDown = false;
	bool m_inSprint = false;

	// path animation state
	bool m_pathActive = false;
	std::vector<lsVector3> m_waypoints;

	static constexpr double kAnimFps = 60.0; ///< keyframe timebase
};

#endif // LSFLYCAM_H
