/**
 * @file lsSky.cpp
 * @brief Procedural sky with day/night cycle (S-series) — see lsSky.h
 */
#include "lsSky.h"

#include <cmath>
#include <iostream>

#include "lsScene.h"
#include "lsObject.h"
#include "lsLight_Direct.h"
#include "../shader/lsShader.h"
#include "../shader/lsShaderManager.h"
#include "../datablocks/lsBatch_Mesh.h"
#include "../datablocks/lsMesh.h"
#include "../assets/lsAsset_Mesh.h"
#include "../assets/lsAssetManager.h"

using std::cout;
using std::endl;

namespace
{
	const float SKY_PI = 3.14159265358979323846f;

	float clampf( float v, float lo, float hi )
	{ return v < lo ? lo : ( v > hi ? hi : v ); }

	float smoothstepf( float e0, float e1, float x )
	{
		float t = clampf( ( x - e0 ) / ( e1 - e0 ), 0.0f, 1.0f );
		return t * t * ( 3.0f - 2.0f * t );
	}

	lsVector3 mixv( const lsVector3& a, const lsVector3& b, float t )
	{
		return lsVector3( a.x + ( b.x - a.x ) * t,
						  a.y + ( b.y - a.y ) * t,
						  a.z + ( b.z - a.z ) * t );
	}
}

lsSky::lsSky()
{
	m_params = computeParams();
}

void lsSky::setTimeOfDay( float hours )
{
	m_timeOfDay = std::fmod( hours, 24.0f );
	if( m_timeOfDay < 0.0f ) m_timeOfDay += 24.0f;
}

void lsSky::update( double deltaSeconds )
{
	if( m_dayLength <= 0.0f ) return;	// frozen
	setTimeOfDay( m_timeOfDay +
		(float)( 24.0 * deltaSeconds / (double)m_dayLength ) );
}

lsSkyParams lsSky::computeParams() const
{
	lsSkyParams p;

	// --- Sun position -------------------------------------------------
	// h = 0 at 06:00 (east horizon), pi/2 at 12:00, pi at 18:00.
	const float h = 2.0f * SKY_PI * ( m_timeOfDay / 24.0f ) - SKY_PI * 0.5f;
	const float tilt = m_sunTiltDeg * SKY_PI / 180.0f;
	const float ce = std::cos( h ), se = std::sin( h );

	// Path in the east-up plane, tilted around the east-west axis so
	// the noon sun leans toward +Z instead of passing through zenith.
	p.sunDir = lsVector3( ce, se * std::cos( tilt ), se * std::sin( tilt ) );
	// already normalized (unit circle rotated); guard against fp noise
	{
		float l = std::sqrt( p.sunDir.x*p.sunDir.x + p.sunDir.y*p.sunDir.y
							+ p.sunDir.z*p.sunDir.z );
		if( l > 1e-6f ) { p.sunDir.x /= l; p.sunDir.y /= l; p.sunDir.z /= l; }
	}
	p.moonDir = lsVector3( -p.sunDir.x, -p.sunDir.y, -p.sunDir.z );

	// --- Cycle factors -------------------------------------------------
	// Daylight ramps through twilight (sun a bit below horizon = dusk).
	p.dayFactor  = smoothstepf( -0.08f, 0.12f, p.sunDir.y );
	// Warm glow band while the sun hugs the horizon (rise or set).
	float hg = 1.0f - std::fabs( p.sunDir.y ) / 0.28f;
	p.sunsetGlow = clampf( hg, 0.0f, 1.0f );
	p.sunsetGlow *= p.sunsetGlow;
	p.starAmount = 1.0f - p.dayFactor;

	// --- Sky colors -----------------------------------------------------
	const lsVector3 dayZenith ( 0.20f, 0.42f, 0.80f );
	const lsVector3 dayHorizon( 0.62f, 0.76f, 0.90f );
	const lsVector3 nightZenith ( 0.008f, 0.012f, 0.030f );
	const lsVector3 nightHorizon( 0.020f, 0.032f, 0.060f );
	const lsVector3 sunsetTint( 1.0f, 0.42f, 0.13f );

	p.zenithColor  = mixv( nightZenith,  dayZenith,  p.dayFactor );
	p.horizonColor = mixv( nightHorizon, dayHorizon, p.dayFactor );
	// warm the horizon (and a touch of zenith) at sunrise/sunset
	p.horizonColor.x += sunsetTint.x * p.sunsetGlow * 0.45f;
	p.horizonColor.y += sunsetTint.y * p.sunsetGlow * 0.45f;
	p.horizonColor.z += sunsetTint.z * p.sunsetGlow * 0.45f;
	p.zenithColor.x  += sunsetTint.x * p.sunsetGlow * 0.08f;
	p.zenithColor.y  += sunsetTint.y * p.sunsetGlow * 0.08f;
	p.zenithColor.z  += sunsetTint.z * p.sunsetGlow * 0.08f;
	// below-horizon haze: darker copy of the horizon
	p.groundColor = lsVector3( p.horizonColor.x * 0.32f,
								p.horizonColor.y * 0.32f,
								p.horizonColor.z * 0.32f );

	// --- Sun color + light intensity ------------------------------------
	// White-hot high in the sky, deep orange at the horizon.
	float heat = 1.0f - clampf( p.sunDir.y, 0.0f, 1.0f );
	heat = heat * heat * heat;
	p.sunColor = mixv( lsVector3( 1.0f, 0.97f, 0.90f ),
					   lsVector3( 1.0f, 0.40f, 0.10f ), heat );
	// Light strength: out at night, dimmed near the horizon.
	p.sunIntensity = smoothstepf( -0.03f, 0.10f, p.sunDir.y )
					 * ( 0.35f + 0.65f * clampf( p.sunDir.y * 2.5f, 0.0f, 1.0f ) );

	// --- Moon -------------------------------------------------------------
	// Visible when it is up AND the sun is down.
	p.moonIntensity = smoothstepf( 0.02f, 0.15f, p.moonDir.y )
					  * ( 1.0f - p.dayFactor );

	return p;
}

bool lsSky::init( lsScene* scene, lsShaderManager* shaders, lsRender* render,
				  const char* meshPath )
{
	if( ! scene || ! shaders || ! render )
	{
		cout << "ERROR: lsSky::init : null scene/shaders/render" << endl;
		return false;
	}
	m_scene = scene;

	// Shader (shared via the shader manager if already loaded)
	m_shader = shaders->Find_Shader( "sky" );
	if( ! m_shader )
	{
		m_shader = new lsShader( render );
		if( ! m_shader )
		{
			cout << "ERROR: lsSky::init : can't create lsShader" << endl;
			return false;
		}
		m_shader->Load_Shader( "sky" );
		m_shader->setKind( ShaderKind::Sky );
		shaders->Add_Shader( m_shader );
	}

	// Sky object: inward-facing sphere, drawn first with no depth write
	lsAsset_Mesh* meshAsset = scene->getAssetManager()->CreateAsset_Mesh( meshPath );
	if( meshAsset ) meshAsset->AllocAsset();

	m_skyObject = new lsObject( scene );
	if( ! m_skyObject )
	{
		cout << "ERROR: lsSky::init : can't create sky object" << endl;
		return false;
	}
	m_skyObject->setName( "SkyObject" );
	m_skyObject->Set_Asset( meshAsset );
	m_skyObject->Set_Scale( 1.0f, 1.0f, 1.0f );
	m_skyObject->Set_Pos( 0, 0, 0 );
	m_skyObject->Set_Rot( 0, 0, 0 );
	if( m_skyObject->Get_Mesh() && m_skyObject->Get_Mesh()->getBatch_list() )
		static_cast<lsBatch_Mesh*>( m_skyObject->Get_Mesh()->getBatch_list() )
			->setShader( m_shader );

	scene->setSkyObject( m_skyObject );
	return true;
}

void lsSky::apply()
{
	m_params = computeParams();

	// Shader uniforms (uploaded in lsShader::UseShader, ShaderKind::Sky)
	if( m_shader ) m_shader->setSkyParams( m_params );

	// Directional light: sun by day, pale moon at night.
	if( m_sun )
	{
		const bool day = m_params.sunIntensity > 0.001f;
		const lsVector3& dir = day ? m_params.sunDir : m_params.moonDir;

		// Engine forward convention: a node's look direction is the -Z
		// column of R = Rx(pitch)*Ry(yaw) — for a light that means
		// dir = ( sin yaw, -cos yaw sin pitch, cos yaw cos pitch ).
		const float rad = SKY_PI / 180.0f;
		float yaw = std::atan2( dir.x, dir.z );
		float sinPitch = -dir.y / std::max( std::cos( yaw ), 1e-4f );
		float pitch = std::asin( clampf( sinPitch, -1.0f, 1.0f ) );
		m_sun->Set_Rot( pitch / rad, yaw / rad, 0.0f );

		if( day )
		{
			m_sun->diffuse = lsVector4( m_params.sunColor.x,
				m_params.sunColor.y, m_params.sunColor.z, 1.0f );
			m_sun->specular = lsVector4( m_params.sunColor.x,
				m_params.sunColor.y, m_params.sunColor.z, 1.0f );
			m_sun->setIntensity( m_params.sunIntensity );
		} else
		{
			// faint blue moonlight so night scenes keep shape
			m_sun->diffuse  = lsVector4( 0.45f, 0.55f, 0.75f, 1.0f );
			m_sun->specular = lsVector4( 0.45f, 0.55f, 0.75f, 1.0f );
			m_sun->setIntensity( m_params.moonIntensity * 0.25f );
		}
	}
}
