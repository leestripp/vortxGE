#ifndef vortxGE_lsBatch_Lines_h
#define vortxGE_lsBatch_Lines_h

// math
#include "../math/lsVector2.h"
#include "../math/lsVector3.h"
#include "../math/lsVector4.h"

#include "lsBatch.h"

class lsRender;


class lsBatch_Lines : public lsBatch
{
public:
    lsBatch_Lines( lsRender *render );
    virtual ~lsBatch_Lines();

    void Draw( lsCamera *cam, lsObject *object, lsPass *pass );
    void Update( double delta );

    // Add lines
    void Begin( GLuint nMaxVerts );
    void AddLine( Vector3f verts[2], Vector4f vColours[2] );
    void End();
    void Cleanup();

    // The easy way
    void buildEmpty( GLfloat scale );
    void buildGrid();

    // openGL data - on video card
    void Copy_GLdata();
    void Clear_GLdata();

private:
    GLuint nNumVerts;

    // Data
    GLuint m_Vao;
    GLuint vbo_verts;
    GLuint vbo_colours;

    Vector3f *pVerts;
    Vector4f *pColours;

};


#endif
