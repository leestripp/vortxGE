#ifndef vortxGE_lsAsset_MeshGuide_Empty_h
#define vortxGE_lsAsset_MeshGuide_Empty_h

#include "lsAsset.h"

class lsShaderManager;


/**
 * @brief Creates a Empty
 * 
 * Draws 2 lines from a center point for each axis.
 * X = Red, Y = Green, Z = Blue.
 * 
 * */

class lsAsset_MeshGuide_Empty : public lsAsset
{
public:
    lsAsset_MeshGuide_Empty( lsOpenGL *opengl );
    virtual ~lsAsset_MeshGuide_Empty();
    
    // lsAsset
    void AllocAsset();
    void FreeAsset();
    
private:
    
};


#endif
