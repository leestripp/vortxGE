/**
 * @file lsAsset_Texture.h
 * @brief Texture asset for image loading
 * 
 * Asset type for loading and managing 2D textures from image files.
 * Supports various image formats including TGA, PNG, JPG.
 * 
 * @author vortxGE
 * @date 2024
 * @see lsAsset
 * @see lsAssetManager
 * @see lsTexture
 */

#ifndef vortxGE_lsAsset_Texture_h
#define vortxGE_lsAsset_Texture_h

#include "lsAsset.h"

/**
 * @class lsAsset_Texture
 * @brief Asset for loading 2D textures
 * 
 * Represents a 2D image texture that can be loaded from file
 * and used for material shading. Textures can contain color,
 * normal, specular, or other texture map data.
 * 
 * @note Asset type: LSAT_TEXTURE
 * @note Uses lsTexture internally to manage GL texture
 * 
 * @example
 * @code
 * lsAssetManager* am = scene->Get_AssetManager();
 * lsAsset* diffuseTex = am->Add_Asset(LSAT_TEXTURE, "textures/brick_diffuse.png");
 * lsAsset* normalTex = am->Add_Asset(LSAT_TEXTURE, "textures/brick_normal.png");
 * @endcode
 */
class lsAsset_Texture : public lsAsset
{
public:
	/**
	 * @brief Construct texture asset
	 * @param render Pointer to renderer
	 */
    lsAsset_Texture( lsRender *render );
    
    /// Destructor
    virtual ~lsAsset_Texture();

    //=========================================================================
    // lsAsset Override
    //=========================================================================
    
    /**
     * @brief Allocate texture from image file
     * 
     * Loads image data from file and creates OpenGL texture.
     */
    void AllocAsset();
    
    /**
     * @brief Free texture data
     * 
     * Deletes OpenGL texture.
     */
    void FreeAsset();

    //=========================================================================
    // Properties
    //=========================================================================
    
    /**
     * @brief Get texture filename
     * @return Path to image file
     */
    const string& getFilename() const
    {
        return filename;
    }

    /**
     * @brief Set texture filename
     * @param val Path to image file
     */
    void setFilename( const string& val )
    {
        filename = val;
    }

private:
    /// Path to image file
    string filename;

};


#endif
