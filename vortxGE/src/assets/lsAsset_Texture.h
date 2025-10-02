#ifndef vortxGE_lsAsset_Texture_h
#define vortxGE_lsAsset_Texture_h

#include "lsAsset.h"


/**
 * @brief Creates a GL texture
 * 
 * Loads and unloads a GL texture.
 * 
 * */
class lsAsset_Texture : public lsAsset
{
public:
    lsAsset_Texture( lsRender *render );
    virtual ~lsAsset_Texture();

    // lsAsset
    void AllocAsset();
    void FreeAsset();

    // Data needed to alloc asset

    const string& getFilename() const
    {
        return filename;
    }

    void setFilename( const string& val )
    {
        filename = val;
    }

private:
    // Data needed to alloc asset
    string filename;

};


#endif
