#pragma once

#include <vector>

#include "lsLight.h"


/**
 * @brief L3.1: fit a spot shadow frustum's depth range to content
 *
 * Pure function (doctestable). Transforms world-space positions into
 * light view space with lightView, expands each depth by its radius,
 * and returns the tight [nearZ, farZ] bracketing all entries.
 *
 * @param lightView  world -> light view matrix (light looks down -Z)
 * @param worldPositions  object world origins
 * @param radii  per-object radius proxy (caller caps huge meshes)
 * @param outNear  fitted near plane (clamped >= 0.05)
 * @param outFar   fitted far plane (>= outNear + 0.1)
 * @return false when the position list is empty (keep current range)
 */
bool lsFitSpotDepthRange( const lsMatrix4x4& lightView,
                          const std::vector<lsVector3>& worldPositions,
                          const std::vector<float>& radii,
                          float& outNear, float& outFar );

class lsLight_Spot : public lsLight
{
public:
    lsLight_Spot(lsNode* parent);
    virtual ~lsLight_Spot();

    // Save data
    const string& toString() override;

    /// L3.1: keep the shadow frustum FOV matched to the outer cone
    /// (cone angles are public members and can change at runtime;
    /// near/far are preserved). Called before the shadow matrix build.
    void syncShadowFrustum() override;

private:

};
