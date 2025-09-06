#ifndef LSASSET_QUAD_H
#define LSASSET_QUAD_H

#include "lsAsset.h"

class lsBatch_Quad;


/**
 * @brief Creates a Quad
 * 
 * Draws a Quad with texture coords for use with render passes.
 * 
 * */
class lsAsset_Quad : public lsAsset
{
public:
    lsAsset_Quad( lsOpenGL *opengl );
    virtual ~lsAsset_Quad();

    // lsAsset
    void AllocAsset();
    void FreeAsset();
    // Set to your lsFBO texture
    void Set_texture( GLuint val );

    // Needed for render pipeline.
    lsBatch_Quad* getBatch_Quad() const
    {
        return batch_quad;
    }

private:
    lsBatch_Quad *batch_quad;

};

#endif // LSASSET_QUAD_H
