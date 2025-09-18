#pragma once

#include "lsAsset.h"

class lsAsset_Mesh;


class lsAsset_BulletShape_ConvexHull : public lsAsset
{
public:
    lsAsset_BulletShape_ConvexHull(lsAsset_Mesh* meshAsset);
    virtual ~lsAsset_BulletShape_ConvexHull();

    void AllocAsset() override;
    void FreeAsset() override;

private:
    lsAsset_Mesh* m_meshAsset;
};
