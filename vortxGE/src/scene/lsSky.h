/**
 * @file lsSky.h
 * @brief Procedural sky with day/night cycle (S-series)
 *
 * lsSky replaces the static HDRI skybox with a procedural atmosphere:
 * time-of-day driven sun position, sky gradients, sun/moon discs and
 * stars (rendered by the "sky" shader, ShaderKind::Sky), plus driving
 * the scene's directional light (direction, color, intensity) so the
 * whole scene lighting follows the cycle.
 *
 * All astronomy/color math lives in computeParams() — a pure function
 * of the current time — so it is fully covered by doctests. The
 * shader only renders the parameters it is handed.
 *
 * @section usage Usage
 * @code
 * lsSky* sky = new lsSky();
 * sky->setTimeOfDay(10.0f);      // 10:00
 * sky->setDayLength(600.0f);     // full cycle in 10 min (0 = frozen)
 * sky->init(scene, shaderManager, render);
 * sky->attachSunLight(sunLight); // lsLight_Direct driven every frame
 * // per frame:
 * sky->update(deltaSeconds);
 * sky->apply();
 * @endcode
 */
#pragma once

#include "../math/lsVector3.h"
#include "../math/lsVector4.h"

class lsScene;
class lsShader;
class lsObject;
class lsShaderManager;
class lsRender;
class lsLight_Direct;

/**
 * @struct lsSkyParams
 * @brief Everything the sky shader + sun light need for one moment.
 *
 * Directions are normalized world-space and point FROM the scene
 * TOWARD the body (sunDir.y > 0 means the sun is above the horizon).
 */
struct lsSkyParams
{
	lsVector3	sunDir;			///< direction toward the sun
	lsVector3	sunColor;		///< sun disc / light color
	float		sunIntensity;	///< directional light strength (0 at night)
	lsVector3	moonDir;		///< direction toward the moon
	float		moonIntensity;	///< moon disc brightness (0 by day)
	lsVector3	zenithColor;	///< sky color straight up
	lsVector3	horizonColor;	///< sky color at the horizon
	lsVector3	groundColor;	///< haze color below the horizon
	float		dayFactor;		///< 0 = night, 1 = full day
	float		starAmount;		///< star visibility 0..1
	float		sunsetGlow;		///< warm horizon glow 0..1
};

/**
 * @class lsSky
 * @brief Day/night cycle state + scene integration.
 *
 * Solar model (deliberately simple, not astronomical):
 * hour angle h = 2*pi*(t/24) - pi/2, so the sun rises in the east
 * (+X) at 06:00, passes overhead at 12:00 and sets in the west at
 * 18:00. The path is tilted around the east-west axis by
 * m_sunTiltDeg so the noon sun sits off-zenith (latitude feel).
 */
class lsSky
{
public:
	lsSky();
	~lsSky();

	/** @brief Set time of day in hours, wrapped to [0, 24) */
	void	setTimeOfDay( float hours );
	float	getTimeOfDay() const { return m_timeOfDay; }

	/** @brief Real seconds per full day cycle; 0 freezes time (default) */
	void	setDayLength( float seconds ) { m_dayLength = seconds; }
	float	getDayLength() const { return m_dayLength; }

	/** @brief Tilt of the sun's path around the east-west axis (degrees) */
	void	setSunTilt( float degrees ) { m_sunTiltDeg = degrees; }
	float	getSunTilt() const { return m_sunTiltDeg; }

	/** @brief Advance the cycle (no-op when day length is 0) */
	void	update( double deltaSeconds );

	/** @brief Pure: compute all sky parameters for the current time */
	lsSkyParams	computeParams() const;

	/** @brief Create the "sky" shader + sky object and register it
	 *         as the scene's sky object (drawn first, no depth write).
	 *         The sky sphere mesh is generated in code — no mesh file. */
	bool	init( lsScene* scene, lsShaderManager* shaders, lsRender* render );

	/** @brief Directional light driven by the cycle (sun by day,
	 *         pale moon at night). Pass nullptr to detach. */
	void	attachSunLight( lsLight_Direct* sun ) { m_sun = sun; }

	/** @brief Push current params to the sky shader + attached light.
	 *         Call once after init and after every update(). */
	void	apply();

	const lsSkyParams&	getParams() const { return m_params; }
	lsObject*			getSkyObject() const { return m_skyObject; }
	lsShader*			getShader() const { return m_shader; }

private:
	/** @brief Procedural UV sphere (positions only), radius kSkyRadius */
	class lsMesh*	buildSphereMesh( lsRender* render );

	float	m_timeOfDay	= 10.0f;	///< hours [0, 24)
	float	m_dayLength	= 0.0f;		///< seconds per cycle, 0 = frozen
	float	m_sunTiltDeg = 23.5f;	///< sun path tilt

	lsScene*		m_scene		= nullptr;
	lsShader*		m_shader	= nullptr;
	lsObject*		m_skyObject	= nullptr;
	lsLight_Direct*	m_sun		= nullptr;
	class lsAsset_Mesh* m_meshAsset	= nullptr;	///< owned sphere asset
	lsSkyParams		m_params;
};
