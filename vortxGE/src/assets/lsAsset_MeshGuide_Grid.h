#ifndef vortxGE_lsAsset_MeshGuide_Grid_h
#define vortxGE_lsAsset_MeshGuide_Grid_h

#include "lsAsset.h"

class lsShaderManager;


/**
 * @brief Creates a Grid floor.
 * 
 * Draws a debug grid floor.
 * 
 * */

class lsAsset_MeshGuide_Grid : public lsAsset
{
public:
    lsAsset_MeshGuide_Grid( lsOpenGL *opengl );
    virtual ~lsAsset_MeshGuide_Grid();

    // lsAsset
    void AllocAsset();
    void FreeAsset();

private:

};


#endif
