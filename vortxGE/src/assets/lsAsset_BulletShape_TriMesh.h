#pragma once

#include "lsAsset.h"

class lsAsset_Mesh;

/**
 * @brief Creates a bullet physics triangle mesh shape from a mesh asset.
 *
 */
class lsAsset_BulletShape_TriMesh : public lsAsset
{
public:
    lsAsset_BulletShape_TriMesh(lsAsset_Mesh* meshAsset);
    virtual ~lsAsset_BulletShape_TriMesh();

    void AllocAsset() override;
    void FreeAsset() override;

private:
    lsAsset_Mesh* m_meshAsset;
};
