/**
 * @file lsFlyCam.h
 * @brief Free-flying camera with keyboard control and scripted flyover paths
 *
 * lsFlyCam is a drop-in lsCamera for scene surveying / debugging:
 *  - WASD-style keyboard flying (forward/back/strafe/vertical), sprint
 *  - mouse-look via incremental deltas
 *  - scripted animations for automated captures: circular orbits and
 *    waypoint tours, advanced per-frame in Move()
 *
 * The demo wires it up with env hooks (VORTX_FLYCAM, VORTX_FLYPATH, ...)
 * so a whole map can be surveyed in one headless run.
 */
#ifndef LSFLYCAM_H
#define LSFLYCAM_H

#include "lsCamera.h"
#include "../math/lsVector3.h"

#include <vector>

class lsFlyCam : public lsCamera
{
public:
	lsFlyCam( lsNode* parent );
	virtual ~lsFlyCam();

	//=========================================================================
	// Movement / look (input-driven, consumed by Move)
	//=========================================================================

	/**
	 * @brief Set flight speed in metres/second (sprint multiplies x4).
	 */
	void setFlySpeed( float mps ) { m_flySpeed = mps; }
	float getFlySpeed() const { return m_flySpeed; }

	/**
	 * @brief Queue one frame of keyboard flight input.
	 *
	 * Stored as flags; Move() integrates them with the frame delta, so
	 * movement speed is frame-rate independent. jump/crouch map to
	 * vertical up/down (reusing the rig's key set).
	 */
	void HandleFlyInput( bool fwd, bool back, bool left, bool right,
						 bool up, bool down, bool sprint );

	/**
	 * @brief Mouse-look deltas (pixels). Positive dx turns right.
	 */
	void HandleFlyLook( float dx, float dy );

	/**
	 * @brief Absolute orientation (degrees): pitch, yaw.
	 */
	void setYawPitch( float yawDeg, float pitchDeg );

	//=========================================================================
	// Scripted animation (VORTX_FLYPATH)
	//=========================================================================

	/**
	 * @brief Fly a horizontal circle around a point.
	 * @param centre     Orbit centre (y = flight altitude)
	 * @param radius     Orbit radius (m)
	 * @param periodSec  Seconds per revolution
	 * @param pitchDeg   Camera pitch while orbiting (negative = look down)
	 * @param lookAtCentre Face the centre instead of the travel direction
	 */
	void Start_Orbit( lsVector3 centre, float radius, float periodSec,
					  float pitchDeg, bool lookAtCentre );

	/**
	 * @brief Fly a waypoint tour at flySpeed, looping. Faces travel
	 *        direction with the given pitch (negative = look down).
	 */
	void Add_Waypoint( lsVector3 pos );
	void Start_Waypoints( float pitchDeg );

	/** @brief Stop any running path animation. */
	void Stop_Path();

	//=========================================================================
	// Virtual Overrides
	//=========================================================================

	/**
	 * @brief Advance input motion + path animation, then base camera update.
	 * @param delta Frame delta in MILLISECONDS (engine convention).
	 */
	virtual void Move( lsScene* scn, lsRender* render, double delta ) override;

private:
	// camera basis from the engine's own ModelMatrix (column-major):
	// forward = -col2, right = col0, up = col1 — no sign guessing.
	lsVector3 forwardVec() const;
	lsVector3 rightVec() const;

	float m_flySpeed = 40.0f;   ///< m/s
	float m_yawDeg = 0.0f;
	float m_pitchDeg = 0.0f;

	// input flags (set per frame by the app, consumed in Move)
	bool m_inFwd = false, m_inBack = false, m_inLeft = false,
		 m_inRight = false, m_inUp = false, m_inDown = false,
		 m_inSprint = false;

	// path animation
	enum class PathMode { None, Orbit, Waypoints };
	PathMode m_pathMode = PathMode::None;
	double m_pathTimeMs = 0.0;
	// orbit params
	lsVector3 m_orbitCentre;
	float m_orbitRadius = 0.0f;
	float m_orbitPeriod = 60.0f;
	float m_pathPitch = -35.0f;
	bool m_orbitLookAt = false;
	// waypoint params
	std::vector<lsVector3> m_waypoints;
	size_t m_wpIndex = 0;
	lsVector3 m_wpFrom;
	double m_wpLegMs = 0.0;
	double m_wpLegDurMs = 1.0;
};

#endif // LSFLYCAM_H
