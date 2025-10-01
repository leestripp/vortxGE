#include "../render/lsOpenGL.h"

// datablocks
#include "../datablocks/lsBatch_Quad.h"
#include "../datablocks/lsMesh.h"

// shader
#include "../shader/lsShaderManager.h"
#include "../shader/lsShader.h"

#include "lsAsset_Quad.h"

lsAsset_Quad::lsAsset_Quad( lsOpenGL *opengl ) : lsAsset(opengl)
{
    setAsset_type( LSAT_QUAD );
    setAsset_name( "Quad Mesh" );

    batch_quad = NULL;

}

lsAsset_Quad::~lsAsset_Quad()
{
    FreeAsset();
}


/*!
 * \brief lsAsset_Quad::AllocAsset
 *
 * creates a Quad for easy texture or pass renders.\n
 *
 */
void lsAsset_Quad::AllocAsset()
{
#ifdef VORTX_DEBUG_ASSETS
    cout << "lsAsset_Quad::AllocAsset()" << endl;
#endif

    lsShader *shader;
    lsMesh *mesh;

    // Create Lines batch
    batch_quad = new lsBatch_Quad( getOpenGL() );
    if( batch_quad )
    {
        // No need to build batch. Already done by lsBatch_Quad.

        // set shader, lsBatch_Lines needs the debug shader
        shader = getOpenGL()->getShaderManager()->Find_Shader( "quad" );
        if( shader )
        {
            batch_quad->setShader( shader );
        } else
        {
            cout << "ERROR: Shader not found - lsAsset_Quad::AllocAsset()" << endl;
        }
        // Create mesh to store Batch
        mesh = new lsMesh( getOpenGL() );
        if( mesh )
        {
            // Add Batch to Mesh
            mesh->Add_Batch( batch_quad );
            // Store Mesh in Asset
            setData( mesh );

        } else
        {
            cout << "ERROR: lsMesh instance failed - lsAsset_Quad::AllocAsset()" << endl;
        }
    }// if batch_quad
}

void lsAsset_Quad::FreeAsset()
{
    if( getData() )
    {
        delete getData();
        setData( NULL );
        batch_quad = NULL;
    }
}

void lsAsset_Quad::Set_texture( GLuint val )
{
    if( batch_quad )
    {
        batch_quad->setTex_albedo( val );
    }
}

