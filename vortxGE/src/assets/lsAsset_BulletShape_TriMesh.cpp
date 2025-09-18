#include "lsAsset_BulletShape_TriMesh.h"
#include "lsAsset_Mesh.h"
#include "../datablocks/lsBulletShape.h"
#include "../datablocks/lsMesh.h"

lsAsset_BulletShape_TriMesh::lsAsset_BulletShape_TriMesh(lsAsset_Mesh* meshAsset)
    : lsAsset(nullptr), m_meshAsset(meshAsset)
{
    setAsset_type(LSAT_BULLETSHAPE_TRIMESH);
    if (m_meshAsset) {
        setAsset_name(m_meshAsset->getAsset_name() + "_trimesh");
    }
}

lsAsset_BulletShape_TriMesh::~lsAsset_BulletShape_TriMesh()
{
    FreeAsset();
}

void lsAsset_BulletShape_TriMesh::AllocAsset()
{
    if (!m_meshAsset || !m_meshAsset->getData()) {
        std::cout << "ERROR: lsAsset_BulletShape_TriMesh::AllocAsset() - Mesh asset is not available." << std::endl;
        return;
    }
    FreeAsset();

    lsBulletShape* bulletshape = new lsBulletShape(getOpenGL());
    if (bulletshape) {
        bulletshape->Make_TriMesh_FromMesh(static_cast<lsMesh*>(m_meshAsset->getData()));
        setData(bulletshape);
    } else {
        std::cout << "ERROR: Failed to allocate asset : " << getAsset_name() << std::endl;
    }
}

void lsAsset_BulletShape_TriMesh::FreeAsset()
{
    if (getData()) {
        delete getData();
        setData(nullptr);
    }
}
