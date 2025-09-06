#ifndef LSMESH_FACE_H
#define LSMESH_FACE_H

#include <string>

const int LSFT_NULL     =   0;
const int LSFT_QUAD     =   1;
const int LSFT_TRIANGLE =   2;

class lsMesh_Vertex;
class lsMesh_UV;
class lsMesh_Normal;

using namespace std;

//! Holds face data
/*!
*
*/
class lsMesh_Face
{
public:
    lsMesh_Face();
    virtual ~lsMesh_Face();

    // list
    lsMesh_Face* getNext_face();
    void setNext_face( lsMesh_Face* face );
    // settings
    int getFace_type();
    void setFace_type( int ft );
    int getIndex();
    void setIndex( int i );

    // Get set
    lsMesh_Vertex *getV1() const;
    void setV1(lsMesh_Vertex *value);
    lsMesh_Vertex *getV2() const;
    void setV2(lsMesh_Vertex *value);
    lsMesh_Vertex *getV3() const;
    void setV3(lsMesh_Vertex *value);
    lsMesh_Vertex *getV4() const;
    void setV4(lsMesh_Vertex *value);
    // UVs
    lsMesh_UV *getUv1() const;
    void setUv1(lsMesh_UV *value);
    lsMesh_UV *getUv2() const;
    void setUv2(lsMesh_UV *value);
    lsMesh_UV *getUv3() const;
    void setUv3(lsMesh_UV *value);
    lsMesh_UV *getUv4() const;
    void setUv4(lsMesh_UV *value);
    // Normals
    lsMesh_Normal *getNorm1() const;
    void setNorm1(lsMesh_Normal *value);
    lsMesh_Normal *getNorm2() const;
    void setNorm2(lsMesh_Normal *value);
    lsMesh_Normal *getNorm3() const;
    void setNorm3(lsMesh_Normal *value);
    lsMesh_Normal *getNorm4() const;
    void setNorm4(lsMesh_Normal *value);

    // Save
    string toString();

private:
    lsMesh_Face* next_face;
    int index;
    int face_type; // Quad or Triangle

    // verts
    lsMesh_Vertex* v1;
    lsMesh_Vertex* v2;
    lsMesh_Vertex* v3;
    lsMesh_Vertex* v4;

    // UVs
    lsMesh_UV* uv1;
    lsMesh_UV* uv2;
    lsMesh_UV* uv3;
    lsMesh_UV* uv4;

    // Normals
    lsMesh_Normal* norm1;
    lsMesh_Normal* norm2;
    lsMesh_Normal* norm3;
    lsMesh_Normal* norm4;

};

#endif // LSMESH_FACE_H
