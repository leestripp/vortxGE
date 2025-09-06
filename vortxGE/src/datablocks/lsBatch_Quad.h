#ifndef vortxGE_lsBatch_Quad_h
#define vortxGE_lsBatch_Quad_h

// OpenGL
#include "../opengl/lsOpenGL.h"

// math
#include "../math/lsVector2.h"
#include "../math/lsVector3.h"
#include "../math/lsVector4.h"

#include "lsBatch.h"


class lsTexture;

//! Draw a quad
/**
 VBO Quad with texture coords.
 \n
 Creates and draws a 2x2 unit quad object. Pivot at center.

 */
class lsBatch_Quad : public lsBatch
{
public:
    lsBatch_Quad( lsOpenGL *opengl );
    virtual ~lsBatch_Quad();

    void Draw( lsCamera *cam, lsObject *object, lsPass *pass );
    void Update( double delta );

    // openGL data - on video card
    void Copy_GLdata();
    void Clear_GLdata();

    GLuint getTex_diffuse() const
    {
        return tex_diffuse;
    }

    void setTex_diffuse( GLuint val )
    {
        tex_diffuse = val;
    }

private:
    GLuint vao_quad;
	// Data
	GLuint vbo_verts;
	GLuint vbo_uvs;
	GLuint vbo_indices;

    // Textures
    GLuint      tex_diffuse;

};


#endif
