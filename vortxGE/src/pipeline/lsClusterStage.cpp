#include "lsClusterStage.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>

#include "../scene/lsScene.h"
#include "../scene/lsCamera.h"
#include "../scene/lsLight.h"
#include "../scene/lsLightManager.h"
#include "../math/lsMath.h"

lsClusterStage::lsClusterStage( lsScene* scene, lsCamera* camera,
                                lsRender* render )
    : m_scene( scene ), m_camera( camera )
{
    (void)render;
    m_disabled = std::getenv( "VORTX_NOCLUSTER" ) != nullptr;
    m_gpu.tilePixels = 32.0f; // CPU build: coarser tiles (16px is GPU-compute territory)
}

lsClusterStage::~lsClusterStage()
{
    // textures are owned by the render backend's retire queue after
    // re-uploads; nothing to destroy explicitly here
}

void lsClusterStage::execute( lsRender* render, lsTargetPool& targets )
{
    // follow the scene's active camera per-frame when no explicit
    // camera is set (same rule as lsSceneStage)
    lsCamera* cam = m_camera;
    if( !cam && m_scene ) cam = m_scene->getActiveCamera();

    if( m_disabled || !m_scene || !cam )
    {
        m_gpu.enabled = false;
        if( m_scene ) m_scene->setClusterGpuData( &m_gpu );
        return;
    }

    const int vpW = targets.width();
    const int vpH = targets.height();
    if( vpW <= 0 || vpH <= 0 )
    {
        m_gpu.enabled = false;
        m_scene->setClusterGpuData( &m_gpu );
        return;
    }

    const float nearZ = cam->getCam_near();
    const float farZ  = cam->getCam_far();

    // (re)setup on viewport change
    if( vpW != m_lastW || vpH != m_lastH )
    {
        const float tanY = (float)std::tan( cam->getCam_fov() * 0.5 * ANG2RAD );
        const float tanX = tanY * (float)vpW / (float)vpH;
        m_grid.setup( vpW, vpH, m_gpu.tilePixels, nearZ, farZ, 12, tanX, tanY ); // CPU build: 32px tiles x 12 slices
        m_lastW = vpW;
        m_lastH = vpH;
    }

    // Refresh active flags HERE so the cluster builder and the
    // draw-time light upload (lsShader::Setup_Lights) agree — the
    // 2026-08-11 dark-floor bug was exactly a stale-active mismatch.
    m_scene->getLightManager()->setActiveLights( cam, 50.0f );

    // Gather active lights in scene-list order. Slot ranks MUST match
    // Setup_Lights' compaction: rank i among active lights == shader
    // light array index i. Ranks 0-2 are the hero slots (legacy shadow
    // path) and are excluded from cluster assignment; direct lights
    // are global and excluded too.
    std::vector<lsClusterLightInput> inputs;
    int rank = 0;
    for( lsLight* light : m_scene->getLightManager()->getLights() )
    {
        if( !light || !light->getActive() ) continue;
        const int slot = rank++;
        if( slot >= 128 ) break;             // shader array cap (E5.2)

        const int type = light->getLight_type();
        if( type == LSLT_DIRECT || slot < 3 ) continue;

        lsClusterLightInput in;
        const lsMatrix4x4 vm = cam->ViewMatrix * light->ModelMatrix;
        in.position = lsVector3( vm.matrix[12], vm.matrix[13], vm.matrix[14] );
        // 1/d^2 attenuation: cluster assignment cut off at 10%
        in.radius = lsLight::clusterRadius( light->getIntensity() );
        in.type = type;
        in.lightIndex = slot;
        if( type == LSLT_SPOT )
        {
            lsVector3 dirW = light->ModelMatrix.Rotate( lsVector3( 0.0f, 0.0f, -1.0f ) );
            in.direction = cam->ViewMatrix.Rotate( dirW );
            in.direction.normalize();
            in.cosOuter = (float)std::cos( light->spot_outer_cutoff * ANG2RAD );
        }
        inputs.push_back( in );
    }
    m_grid.build( inputs );

    static const bool s_trace = std::getenv( "VORTX_CLUSTER_TRACE" ) != nullptr;
    if( s_trace )
    {
        static int s_frames = 0;
        if( s_frames++ < 3 || s_frames % 60 == 0 )
        {
            uint32_t nonzero = 0;
            for( int ci = 0; ci < m_grid.clusterCount(); ++ci )
                if( m_grid.grid()[(size_t)ci * 2 + 1] > 0 ) ++nonzero;
            std::cout << "CLUSTER: inputs=" << inputs.size()
                      << " indices=" << m_grid.indices().size()
                      << " nonemptyClusters=" << nonzero << "/" << m_grid.clusterCount()
                      << " grid=" << m_grid.tilesX() << "x" << m_grid.tilesY()
                      << "x" << m_grid.slices() << std::endl;
        }
    }

    // create the two textures once
    if( !m_gpu.gridTex )
    {
        render->GenTextures( 1, &m_gpu.gridTex );
        render->GenTextures( 1, &m_gpu.indexTex );
        const lsTextureID ids[2] = { m_gpu.gridTex, m_gpu.indexTex };
        for( int t = 0; t < 2; ++t )
        {
            render->BindTexture( LS_TEXTURE_2D, ids[t] );
            render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_MIN_FILTER, LS_NEAREST );
            render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_MAG_FILTER, LS_NEAREST );
            render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_WRAP_S, LS_CLAMP_TO_EDGE );
            render->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_WRAP_T, LS_CLAMP_TO_EDGE );
        }
        render->BindTexture( LS_TEXTURE_2D, 0 );
    }

    // grid texture: tilesX wide, tilesY*slices tall (flattened 3D)
    render->ActiveTexture( LS_TEXTURE0 + 12 );
    render->BindTexture( LS_TEXTURE_2D, m_gpu.gridTex );
    render->TexImage2D( LS_TEXTURE_2D, 0, LS_RG32UI,
                        m_grid.tilesX(), m_grid.tilesY() * m_grid.slices(), 0,
                        LS_RG_INTEGER, LS_UNSIGNED_INT, m_grid.grid().data() );

    // index list: 256 wide, as tall as needed
    const auto& idx = m_grid.indices();
    const int idxW = m_gpu.indexTexWidth;
    const int idxH = std::max( 1, ( (int)idx.size() + idxW - 1 ) / idxW );
    m_indexPad.assign( (size_t)idxW * (size_t)idxH, 0u );
    if( !idx.empty() )
        std::copy( idx.begin(), idx.end(), m_indexPad.begin() );
    render->ActiveTexture( LS_TEXTURE0 + 13 );
    render->BindTexture( LS_TEXTURE_2D, m_gpu.indexTex );
    render->TexImage2D( LS_TEXTURE_2D, 0, LS_R32UI, idxW, idxH, 0,
                        LS_RED_INTEGER, LS_UNSIGNED_INT, m_indexPad.data() );

    render->ActiveTexture( LS_TEXTURE0 );

    m_gpu.tilesX = m_grid.tilesX();
    m_gpu.tilesY = m_grid.tilesY();
    m_gpu.slices = m_grid.slices();
    m_gpu.nearZ = nearZ;
    m_gpu.farZ = farZ;
    m_gpu.enabled = true;
    m_scene->setClusterGpuData( &m_gpu );
}
