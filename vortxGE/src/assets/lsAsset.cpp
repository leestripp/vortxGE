// core
#include "../vortx_debug.h"
#include "../vortxGE.h"

// Shaders
#include "../shader/lsShaderManager.h"

// datablocks
#include "../datablocks/lsData.h"
#include "../datablocks/lsMesh.h"
#include "../datablocks/lsTexture.h"

#include "lsAssetManager.h"
#include "lsAsset.h"


lsAsset::lsAsset( lsRender *m_render )
{
    // Managers MUST be set by Asset Manager
    assetmanager = NULL;

    // Make sure we have access to render
    render = m_render;

    // Ser our pointers
    next_asset = NULL;
    prev_asset = NULL;

    // Data
    asset_type = LSAT_NONE;
    asset_name = "NOT set";
    users = 0;
    data = NULL;

}

lsAsset::~lsAsset()
{
    // dont delete managers
    // e.g. assetmanager

    if( data ) delete data;
}

void lsAsset::AddUser()
{
    users++;
}

void lsAsset::RemoveUser()
{
    users--;
}

