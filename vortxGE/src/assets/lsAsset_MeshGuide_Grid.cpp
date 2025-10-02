// core
#include "../vortxGE.h"

// shader
#include "../shader/lsShaderManager.h"

// datablocks
#include "../datablocks/lsMesh.h"
#include "../datablocks/lsBatch_Lines.h"

#include "lsAsset_MeshGuide_Grid.h"

lsAsset_MeshGuide_Grid::lsAsset_MeshGuide_Grid( lsRender *render ) : lsAsset(render)
{
    setAsset_type( LSAT_MESHGUIDE_GRID );
}

lsAsset_MeshGuide_Grid::~lsAsset_MeshGuide_Grid()
{
    FreeAsset();
}


// #######################
// lsAsset

void lsAsset_MeshGuide_Grid::AllocAsset()
{
    lsShader *shader;
    lsMesh *mesh;

    // Create Lines batch
    lsBatch_Lines *batch_lines = new lsBatch_Lines( getOpenGL() );
    if( batch_lines )
    {
        // Easy build Empty.
        batch_lines->buildGrid();

        // set shader, lsBatch_Lines needs the debug shader
        shader = getOpenGL()->getShaderManager()->Find_Shader( "debug" );
        if( shader )
        {
            batch_lines->setShader( shader );
        }
        // Create mesh to store Batch
        mesh = new lsMesh( getOpenGL() );
        if( mesh )
        {
            // Add Batch to Mesh
            mesh->Add_Batch( batch_lines );
            // Store Mesh in Asset
            setData( mesh );
        }
    }
}

void lsAsset_MeshGuide_Grid::FreeAsset()
{
    if( getData() )
    {
        delete getData();
        setData( NULL );
    }
}

