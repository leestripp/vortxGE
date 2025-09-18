#include "lsAsset_BulletShape_ConvexHull.h"
#include "lsAsset_Mesh.h"
#include "../datablocks/lsBulletShape.h"
#include "../datablocks/lsMesh.h"

lsAsset_BulletShape_ConvexHull::lsAsset_BulletShape_ConvexHull(lsAsset_Mesh* meshAsset)
    : lsAsset(nullptr), m_meshAsset(meshAsset)
{
    setAsset_type(LSAT_BULLETSHAPE_CONVEXHULL);
    if (m_meshAsset) {
        setAsset_name(m_meshAsset->getAsset_name() + "_convexhull");
    }
}

lsAsset_BulletShape_ConvexHull::~lsAsset_BulletShape_ConvexHull()
{
    FreeAsset();
}

void lsAsset_BulletShape_ConvexHull::AllocAsset()
{
    if (!m_meshAsset || !m_meshAsset->getData()) {
        std::cout << "ERROR: lsAsset_BulletShape_ConvexHull::AllocAsset() - Mesh asset is not available." << std::endl;
        return;
    }
    FreeAsset();

    lsBulletShape* bulletshape = new lsBulletShape(getOpenGL());
    if (bulletshape) {
        bulletshape->Make_ConvexHull_FromMesh(static_cast<lsMesh*>(m_meshAsset->getData()));
        setData(bulletshape);
    } else {
        std::cout << "ERROR: Failed to allocate asset : " << getAsset_name() << std::endl;
    }
}

void lsAsset_BulletShape_ConvexHull::FreeAsset()
{
    if (getData()) {
        delete getData();
        setData(nullptr);
    }
}
