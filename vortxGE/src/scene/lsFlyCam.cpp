#include "lsFlyCam.h"

#include "../animation/lsAnimationManager.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace {
constexpr float kDeg2Rad = 3.14159265358979323846f / 180.0f;
constexpr float kRad2Deg = 180.0f / 3.14159265358979323846f;

/// Yaw (degrees) that makes this node's -Z forward face direction
/// (dx,dz): forward(yaw) = (-sin yaw, 0, -cos yaw), so yaw = atan2(-dx,-dz).
float facingYawDeg( float dx, float dz )
{
	return std::atan2( -dx, -dz ) * kRad2Deg;
}
}

lsFlyCam::lsFlyCam( lsNode* parent ) : lsAnimNode( parent )
{
	setName( "FlyCam" );

	// Child camera: carries PITCH only (rotation.x); yaw stays on this
	// node (rotation.y). Combined via the parent chain the orientation
	// is Ry(yaw)*Rx(pitch) — the canonical fly-cam order.
	m_camera = new lsCamera( this );
	m_camera->setName( "FlyCam.camera" );
	Add_Node( m_camera );

	// Keyframe system binds to this node's Position/Rotation/Scale
	// properties; paths are built with InsertKey_Pos/InsertKey_Rot.
	Init_Animation( kAnimFps );
}

lsFlyCam::~lsFlyCam()
{
}

void lsFlyCam::applyYawPitch()
{
	if( m_pitchDeg >  89.0f ) m_pitchDeg =  89.0f;
	if( m_pitchDeg < -89.0f ) m_pitchDeg = -89.0f;
	Set_Rot( 0.0f, m_yawDeg, 0.0f );
	m_camera->Set_Rot( m_pitchDeg, 0.0f, 0.0f );
}

float lsFlyCam::pathHeight( float x, float y, float z ) const
{
	if( ! m_heightQuery ) return y;
	const float floor_ = m_heightQuery( x, z ) + m_pathClearance;
	return y > floor_ ? y : floor_;
}

// VORTX_FLYDEBUG=1: trace path keyframes (terrain lift + timing)
static bool flyDebug()
{
	static const bool on = std::getenv( "VORTX_FLYDEBUG" ) != nullptr;
	return on;
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
		Stop_Path();
}

void lsFlyCam::HandleFlyLook( float dx, float dy )
{
	m_yawDeg   -= dx * 0.15f;
	m_pitchDeg -= dy * 0.15f;
	applyYawPitch();
}

void lsFlyCam::setYawPitch( float yawDeg, float pitchDeg )
{
	m_yawDeg = yawDeg;
	m_pitchDeg = pitchDeg;
	applyYawPitch();
}

void lsFlyCam::Start_Orbit( lsVector3 centre, float radius, float periodSec,
							float pitchDeg, bool lookAtCentre )
{
	if( periodSec < 0.1f ) periodSec = 60.0f;

	constexpr int kKeys = 36;
	const double periodFrames = ( double )periodSec * kAnimFps;
	for( int i = 0; i <= kKeys; ++i )
	{
		const float a = ( float )i / kKeys * 2.0f * ( float )M_PI;
		const float px = centre.x + std::cos( a ) * radius;
		const float pz = centre.z + std::sin( a ) * radius;
		// terrain-following altitude: never dip below surface+clearance
		const float py = pathHeight( px, centre.y, pz );
		const double frame = ( double )i / kKeys * periodFrames;

		float yaw;
		if( lookAtCentre )
			yaw = facingYawDeg( centre.x - px, centre.z - pz );
		else
			yaw = facingYawDeg( -std::sin( a ), std::cos( a ) ); // velocity

		getAnimationManager()->InsertKey_Pos( frame, lsVector3( px, py, pz ) );
		getAnimationManager()->InsertKey_Rot( frame, lsVector3( 0.0f, yaw, 0.0f ) );
	}

	m_yawDeg = 0.0f;   // animation owns rotation.y while playing
	m_pitchDeg = pitchDeg;
	m_camera->Set_Rot( m_pitchDeg, 0.0f, 0.0f );
	m_pathActive = true;
	Play_Animation( true );

	if( flyDebug() )
	{
		// re-evaluate the key heights for the trace (same math as above)
		for( int i = 0; i <= 8; ++i )
		{
			const float a = ( float )i / 8 * 2.0f * ( float )M_PI;
			const float px = centre.x + std::cos( a ) * radius;
			const float pz = centre.z + std::sin( a ) * radius;
			const float t = m_heightQuery ? m_heightQuery( px, pz ) : -1.0f;
			std::fprintf( stderr,
				"FLYDEBUG orbit key %d: pos(%.0f,%.0f,%.0f) terrain=%.1f -> y=%.1f\n",
				i, px, centre.y, pz, t, pathHeight( px, centre.y, pz ) );
		}
	}
}

void lsFlyCam::Add_Waypoint( lsVector3 pos )
{
	m_waypoints.push_back( pos );
}

void lsFlyCam::Start_Waypoints( float pitchDeg )
{
	const size_t n = m_waypoints.size();
	if( n < 2 ) return;

	// terrain-following copies: each key is lifted above the surface
	std::vector<lsVector3> pts( n );
	for( size_t i = 0; i < n; ++i )
	{
		const lsVector3& w = m_waypoints[i];
		pts[i] = lsVector3( w.x, pathHeight( w.x, w.y, w.z ), w.z );
	}

	double frame = 0.0;
	for( size_t i = 0; i <= n; ++i )
	{
		const lsVector3& p = pts[i % n];
		const lsVector3& to = pts[( i + 1 ) % n];
		const float dx = to.x - p.x, dz = to.z - p.z;
		float yaw = rotation.y;
		if( dx * dx + dz * dz > 1e-6f ) yaw = facingYawDeg( dx, dz );

		getAnimationManager()->InsertKey_Pos( frame, p );
		getAnimationManager()->InsertKey_Rot( frame, lsVector3( 0.0f, yaw, 0.0f ) );

		// next leg duration at the (slower) scripted-path cruise speed
		if( i < n )
		{
			const float lx = to.x - p.x, ly = to.y - p.y, lz = to.z - p.z;
			const float leg = std::sqrt( lx*lx + ly*ly + lz*lz );
			frame += ( double )( leg / m_pathSpeed ) * kAnimFps;
			if( frame < ( double )i + 1.0 ) frame = ( double )i + 1.0;
		}
	}

	Set_Pos( pts[0].x, pts[0].y, pts[0].z );
	m_yawDeg = 0.0f;
	m_pitchDeg = pitchDeg;
	m_camera->Set_Rot( m_pitchDeg, 0.0f, 0.0f );
	m_pathActive = true;
	Play_Animation( true );
}

void lsFlyCam::Stop_Path()
{
	if( ! m_pathActive ) return;
	Stop_Animation();
	m_pathActive = false;
	// resume manual control from the pose the animation left behind
	m_yawDeg = rotation.y;
}

void lsFlyCam::Move( lsScene* scn, lsRender* render, double delta )
{
	// delta is milliseconds (engine convention)
	const double dt = delta * 0.001;

	if( ! m_pathActive )
	{
		// input-driven flight: full 6DOF — forward includes pitch (fly
		// where you look), strafe is horizontal, vertical is world Y
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
			const float yaw = m_yawDeg * kDeg2Rad;
			const float pitch = m_pitchDeg * kDeg2Rad;
			const float cy = std::cos( yaw ), sy = std::sin( yaw );
			const float cp = std::cos( pitch ), sp = std::sin( pitch );
			// forward = -Z column of Ry(yaw)*Rx(pitch); right = X column
			const lsVector3 f( -sy * cp, sp, -cy * cp );
			const lsVector3 r( cy, 0.0f, -sy );
			Set_Pos( position.x + ( f.x * fwd + r.x * side ) * speed,
					 position.y + ( f.y * fwd + vert ) * speed,
					 position.z + ( f.z * fwd + r.z * side ) * speed );
		}
	}

	// animation update + transform rebuild (writes position/rotation.y
	// when a path is playing)
	lsAnimNode::Move( scn, render, delta );
}
