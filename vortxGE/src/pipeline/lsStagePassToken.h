/**
 * @file lsStagePassToken.h
 * @brief Minimal lsPass used only as a pass_type token for the legacy
 *        lsObject::Optimize/Draw path (Plan.md Part 3 bridge).
 *
 * lsObject::Optimize dereferences pass unconditionally and
 * lsObject::Draw renders nothing when pass is null, so pipeline stages
 * carry one of these with the pass_type the object path expects
 * (LSPT_FBO for scene stages, LSPT_SHADOWMAP for shadow stages).
 * Removed in Phase R6 when the object path stops taking lsPass.
 */
#pragma once

#include "../passes/lsPass.h"

class lsStagePassToken : public lsPass
{
public:
	lsStagePassToken( lsRender* render, int type ) : lsPass( render )
	{ setPass_type( type ); }
	void Update() override {}
	void Render() override {}
	void Resize( int, int, int, int ) override {}
	const std::string& toString() override
	{ static const std::string s = "stage-token"; return s; }
};
