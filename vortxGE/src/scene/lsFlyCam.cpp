#include "lsFlyCam.h"

#include <cmath>

namespace {
constexpr float kDeg2Rad = 3.14159265358979323846f / 180.0f;
}

lsFlyCam::lsFlyCam( lsNode* parent ) : lsCamera( parent )
{
	setName( "FlyCam" );
}

lsFlyCam::~lsFlyCam()
{
}

lsVector3 lsFlyCam::forwardVec() const
{
	// ModelMatrix is column-major; camera forward = -Z column.
	return lsVector3( -ModelMatrix.matrix[2],
					  -ModelMatrix.matrix[6],
					  -ModelMatrix.matrix[10] );
}

lsVector3 lsFlyCam::rightVec() const
{
	return lsVector3( ModelMatrix.matrix[0],
					  ModelMatrix.matrix[4],
					  ModelMatrix.matrix[8] );
}

void lsFlyCam::HandleFlyInput( bool fwd, bool back, bool left, bool right,
							   bool up, bool down, bool sprint )
{
	m_inFwd = fwd;   m_inBack = back;
	m_inLeft = left; m_inRight = right;
	m_inUp = up;     m_inDown = down;
	m_inSprint = sprint;
	// manual input cancels any running path animation
	if( fwd || back || left || right || up || down )
		m_pathMode = PathMode::None;
}

void lsFlyCam::HandleFlyLook( float dx, float dy )
{
	m_yawDeg   -= dx * 0.15f;
	m_pitchDeg -= dy * 0.15f;
	if( m_pitchDeg >  89.0f ) m_pitchDeg =  89.0f;
	if( m_pitchDeg < -89.0f ) m_pitchDeg = -89.0f;
	Set_Rot( m_pitchDeg, m_yawDeg, 0.0f );
}

void lsFlyCam::setYawPitch( float yawDeg, float pitchDeg )
{
	m_yawDeg = yawDeg;
	m_pitchDeg = pitchDeg;
	if( m_pitchDeg >  89.0f ) m_pitchDeg =  89.0f;
	if( m_pitchDeg < -89.0f ) m_pitchDeg = -89.0f;
	Set_Rot( m_pitchDeg, m_yawDeg, 0.0f );
}

void lsFlyCam::Start_Orbit( lsVector3 centre, float radius, float periodSec,
							float pitchDeg, bool lookAtCentre )
{
	m_orbitCentre = centre;
	m_orbitRadius = radius;
	m_orbitPeriod = periodSec > 0.1f ? periodSec : 60.0f;
	m_pathPitch = pitchDeg;
	m_orbitLookAt = lookAtCentre;
	m_pathTimeMs = 0.0;
	m_pathMode = PathMode::Orbit;
}

void lsFlyCam::Add_Waypoint( lsVector3 pos )
{
	m_waypoints.push_back( pos );
}

void lsFlyCam::Start_Waypoints( float pitchDeg )
{
	if( m_waypoints.size() < 2 ) return;
	m_pathPitch = pitchDeg;
	m_pathTimeMs = 0.0;
	m_wpIndex = 0;
	m_wpFrom = m_waypoints[0];
	m_wpLegMs = 0.0;
	m_wpLegDurMs = m_waypoints[0].distance( m_waypoints[1] )
				 / m_flySpeed * 1000.0;
	if( m_wpLegDurMs < 1.0 ) m_wpLegDurMs = 1.0;
	Set_Pos( m_waypoints[0].x, m_waypoints[0].y, m_waypoints[0].z );
	m_pathMode = PathMode::Waypoints;
}

void lsFlyCam::Stop_Path()
{
	m_pathMode = PathMode::None;
}

void lsFlyCam::Move( lsScene* scn, lsRender* render, double delta )
{
	// delta is milliseconds (engine convention)
	const double dt = delta * 0.001;

	if( m_pathMode == PathMode::Orbit )
	{
		m_pathTimeMs += delta;
		const float ang = ( float )( m_pathTimeMs * 0.001 /
									 ( double )m_orbitPeriod * 2.0 * M_PI );
		const float px = m_orbitCentre.x + std::cos( ang ) * m_orbitRadius;
		const float pz = m_orbitCentre.z + std::sin( ang ) * m_orbitRadius;
		Set_Pos( px, m_orbitCentre.y, pz );

		if( m_orbitLookAt )
		{
			// yaw faces the orbit centre
			m_yawDeg = std::atan2( m_orbitCentre.x - px,
								   m_orbitCentre.z - pz ) / kDeg2Rad;
		}
		else
		{
			// yaw tangential to travel direction (d/dt of position)
			m_yawDeg = std::atan2( -std::sin( ang ),
								    std::cos( ang ) ) / kDeg2Rad + 90.0f;
		}
		Set_Rot( m_pathPitch, m_yawDeg, 0.0f );
	}
	else if( m_pathMode == PathMode::Waypoints && m_waypoints.size() >= 2 )
	{
		m_wpLegMs += delta;
		while( m_wpLegMs >= m_wpLegDurMs )
		{
			m_wpLegMs -= m_wpLegDurMs;
			m_wpFrom = m_waypoints[m_wpIndex];
			m_wpIndex = ( m_wpIndex + 1 ) % m_waypoints.size();
			const lsVector3& to = m_waypoints[m_wpIndex];
			m_wpLegDurMs = m_wpFrom.distance( to ) / m_flySpeed * 1000.0;
			if( m_wpLegDurMs < 1.0 ) m_wpLegDurMs = 1.0;
		}
		const lsVector3& to = m_waypoints[m_wpIndex];
		const float t = ( float )( m_wpLegMs / m_wpLegDurMs );
		const float px = m_wpFrom.x + ( to.x - m_wpFrom.x ) * t;
		const float py = m_wpFrom.y + ( to.y - m_wpFrom.y ) * t;
		const float pz = m_wpFrom.z + ( to.z - m_wpFrom.z ) * t;
		Set_Pos( px, py, pz );
		const float dx = to.x - m_wpFrom.x, dz = to.z - m_wpFrom.z;
		if( dx * dx + dz * dz > 1e-6f )
			m_yawDeg = std::atan2( dx, dz ) / kDeg2Rad;
		Set_Rot( m_pathPitch, m_yawDeg, 0.0f );
	}
	else
	{
		// input-driven flight
		float fwd = 0.0f, side = 0.0f, vert = 0.0f;
		if( m_inFwd )   fwd  += 1.0f;
		if( m_inBack )  fwd  -= 1.0f;
		if( m_inRight ) side += 1.0f;
		if( m_inLeft )  side -= 1.0f;
		if( m_inUp )    vert += 1.0f;
		if( m_inDown )  vert -= 1.0f;

		if( fwd != 0.0f || side != 0.0f || vert != 0.0f )
		{
			const float speed = m_flySpeed * ( m_inSprint ? 4.0f : 1.0f )
							  * ( float )dt;
			const lsVector3 f = forwardVec();
			const lsVector3 r = rightVec();
			const float nx = position.x + ( f.x * fwd + r.x * side ) * speed;
			const float ny = position.y + ( f.y * fwd + r.y * side
											+ vert ) * speed;
			const float nz = position.z + ( f.z * fwd + r.z * side ) * speed;
			Set_Pos( nx, ny, nz );
		}
	}

	lsCamera::Move( scn, render, delta );
}
