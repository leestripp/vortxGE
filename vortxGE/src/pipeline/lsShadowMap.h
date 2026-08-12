/**
 * @file lsShadowMap.h
 * @brief Per-light shadow-map subsystem (Plan.md Part 4, L1.1).
 *
 * Owns a light's depth target (lsFBO_Depth), resolves the map
 * resolution from the light type + quality preset, and wires the
 * light for sampling (texture, has_shadowmap, texel-snapping sizes).
 * lsShadowStage (L1.6) becomes a thin driver over this class.
 */
#pragma once

class lsFBO_Depth;
class lsLight;
class lsRender;

/// Shadow quality presets (L1.6). Selects map resolution per light
/// type; High matches the legacy demo sizes (4096 sun / 2048 spot).
enum class lsShadowQuality { Low = 0, Medium = 1, High = 2 };

class lsShadowMap
{
public:
	/// Creates the depth FBO immediately and self-wires the light.
	/// Resolution = resolutionFor( light, quality ).
	lsShadowMap( lsLight* light, lsShadowQuality quality, lsRender* render );
	~lsShadowMap();

	lsFBO_Depth*	fbo() const { return m_fbo; }
	int				size() const { return m_size; }

	/// Resolution policy: 0 = light type does not cast shadows.
	static int resolutionFor( lsLight* light, lsShadowQuality q );

	/// Global default quality (init: VORTX_SHADOW_QUALITY=low|med|high,
	/// default High). Read by the demo when building shadow stages.
	static void setQuality( lsShadowQuality q );
	static lsShadowQuality getQuality();

private:
	lsLight*		m_light = nullptr;
	lsFBO_Depth*	m_fbo = nullptr;
	int				m_size = 0;
};
