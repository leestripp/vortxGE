// datablocks
#include "../datablocks/lsData.h"
#include "../datablocks/lsTexture.h"
#include "../datablocks/lsStats.h"

#include "lsAsset_Texture.h"

lsAsset_Texture::lsAsset_Texture( lsRender *render ) : lsAsset(render)
{
    setAsset_type( LSAT_TEXTURE );
}

lsAsset_Texture::~lsAsset_Texture()
{
    FreeAsset();
}

// #######################
// lsAsset

/*!
 * \brief lsAsset_Texture::AllocAsset
 *
 * Make sure you set the filename before calling this.\n\
 *
 */
void lsAsset_Texture::AllocAsset()
{
    lsTexture *texture;

    // clean up
    FreeAsset();

    texture = new lsTexture( getOpenGL() );
    if( texture )
    {
        // This needs an fix
        texture->load_image( filename, true, false );

		setData( texture );
		// memory used
		getOpenGL()->getStats()->addUsed_mem( texture->getMem_used() );
		
    }
}

void lsAsset_Texture::FreeAsset()
{
    if( getData() )
    {
		// memory used
		getOpenGL()->getStats()->addUsed_mem( -getData()->getMem_used() );
		
        delete getData();
        setData( NULL );
    }
}

