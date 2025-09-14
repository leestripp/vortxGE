#pragma once

#include "lsAsset.h"

class lsShaderManager;

/**
 * @brief Creates a Mesh Asset
 *
 */
class lsAsset_Mesh : public lsAsset
{
public:
    lsAsset_Mesh( lsOpenGL *opengl );
    virtual ~lsAsset_Mesh();

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
