/**
 * @file lsClusterStage.h
 * @brief L5: per-frame CPU cluster light assignment + GPU upload.
 *
 * Runs before the scene pass: transforms active lights to view space,
 * builds the cluster grid (lsClusterGrid) and uploads it as two
 * integer textures (RG32UI grid + R32UI index list). Shaders read
 * the result through lsScene::getClusterGpuData().
 *
 * Kill switch: VORTX_NOCLUSTER=1 disables (legacy 64-wide loop).
 */
#pragma once

#include <vector>

#include "lsRenderStage.h"
#include "lsTargetPool.h"
#include "../math/lsClusterGrid.h"

class lsScene;
class lsCamera;

class lsClusterStage : public lsRenderStage
{
public:
    lsClusterStage( lsScene* scene, lsCamera* camera, lsRender* render );
    ~lsClusterStage() override;

    Type type() const override { return Type::Prepass; }
    void execute( lsRender* render, lsTargetPool& targets ) override;

    const lsClusterGpuData& data() const { return m_gpu; }

private:
    lsScene*            m_scene;
    lsCamera*           m_camera;
    lsClusterGrid       m_grid;
    lsClusterGpuData    m_gpu;
    int                 m_lastW = 0, m_lastH = 0;
    bool                m_disabled = false;
    std::vector<uint32_t> m_indexPad;   ///< padded index upload buffer
};
