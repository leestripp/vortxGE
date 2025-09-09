#pragma once

#include "lsAsset.h"

/**
 * @brief Creates a bullet physics capsule (pill) shape.
 *
 */
class lsAsset_BulletShape_Pill : public lsAsset
{
public:
    lsAsset_BulletShape_Pill(float radius, float height);
    virtual ~lsAsset_BulletShape_Pill();

    void AllocAsset() override;
    void FreeAsset() override;

private:
    float m_radius;
    float m_height;
};
