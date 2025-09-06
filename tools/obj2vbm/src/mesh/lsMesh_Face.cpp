#include <iostream>
#include <sstream>

// Math
#include "../math/lsVector2.h"
#include "../math/lsVector3.h"

#include "lsMesh_Face.h"
#include "lsMesh_Vertex.h"
#include "lsmesh_normal.h"
#include "lsmesh_uv.h"


lsMesh_Face::lsMesh_Face()
{
    next_face = NULL;
    index = 0;
    face_type = LSFT_NULL; // Quad or Triangle

    // referance pointers to data
    // verts
    v1 = NULL;
    v2 = NULL;
    v3 = NULL;
    v4 = NULL;

    // Normals
    norm1 = NULL;
    norm2 = NULL;
    norm3 = NULL;
    norm4 = NULL;

    // UVs
    uv1 = NULL;
    uv2 = NULL;
    uv3 = NULL;
    uv4 = NULL;

}

lsMesh_Face::~lsMesh_Face()
{
    // DO NOT delete verts, uvs or normals, lsMesh will do that.

    // debug
    // cout << "Delete next face" << endl;
    if( next_face ) delete next_face;
}

lsMesh_Face* lsMesh_Face::getNext_face()
{
    return next_face;
}

void lsMesh_Face::setNext_face( lsMesh_Face* face )
{
    next_face = face;
}


int lsMesh_Face::getFace_type()
{
    return face_type;
}

void lsMesh_Face::setFace_type( int ft )
{
    face_type = ft;
}

int lsMesh_Face::getIndex()
{
    return index;
}

void lsMesh_Face::setIndex( int i )
{
    index = i;
}

lsMesh_Vertex *lsMesh_Face::getV1() const
{
    return v1;
}

void lsMesh_Face::setV1(lsMesh_Vertex *value)
{
    v1 = value;
}

lsMesh_Vertex *lsMesh_Face::getV2() const
{
    return v2;
}

void lsMesh_Face::setV2(lsMesh_Vertex *value)
{
    v2 = value;
}

lsMesh_Vertex *lsMesh_Face::getV3() const
{
    return v3;
}

void lsMesh_Face::setV3(lsMesh_Vertex *value)
{
    v3 = value;
}

lsMesh_Vertex *lsMesh_Face::getV4() const
{
    return v4;
}

void lsMesh_Face::setV4(lsMesh_Vertex *value)
{
    v4 = value;
}

lsMesh_UV *lsMesh_Face::getUv1() const
{
    return uv1;
}

void lsMesh_Face::setUv1(lsMesh_UV *value)
{
    uv1 = value;
}

lsMesh_UV *lsMesh_Face::getUv2() const
{
    return uv2;
}

void lsMesh_Face::setUv2(lsMesh_UV *value)
{
    uv2 = value;
}

lsMesh_UV *lsMesh_Face::getUv3() const
{
    return uv3;
}

void lsMesh_Face::setUv3(lsMesh_UV *value)
{
    uv3 = value;
}

lsMesh_UV *lsMesh_Face::getUv4() const
{
    return uv4;
}

void lsMesh_Face::setUv4(lsMesh_UV *value)
{
    uv4 = value;
}

lsMesh_Normal *lsMesh_Face::getNorm1() const
{
    return norm1;
}

void lsMesh_Face::setNorm1(lsMesh_Normal *value)
{
    norm1 = value;
}

lsMesh_Normal *lsMesh_Face::getNorm2() const
{
    return norm2;
}

void lsMesh_Face::setNorm2(lsMesh_Normal *value)
{
    norm2 = value;
}

lsMesh_Normal *lsMesh_Face::getNorm3() const
{
    return norm3;
}

void lsMesh_Face::setNorm3(lsMesh_Normal *value)
{
    norm3 = value;
}

lsMesh_Normal *lsMesh_Face::getNorm4() const
{
    return norm4;
}

void lsMesh_Face::setNorm4(lsMesh_Normal *value)
{
    norm4 = value;
}

string lsMesh_Face::toString()
{
    stringstream ss;

    // index
    ss << index;
    ss << " ";

    // V1
    ss << v1->getIndex();
    ss << "/";
    ss << norm1->getNindex();
    ss << "/";
    ss << uv1->getUv_index();
    ss << " ";

    // V2
    ss << v2->getIndex();
    ss << "/";
    ss << norm2->getNindex();
    ss << "/";
    ss << uv2->getUv_index();
    ss << " ";

    // V3
    ss << v3->getIndex();
    ss << "/";
    ss << norm3->getNindex();
    ss << "/";
    ss << uv3->getUv_index();

    if( face_type == LSFT_QUAD )
    {
        // Quad
        ss << " ";
        ss << v4->getIndex();
        ss << "/";
        ss << norm4->getNindex();
        ss << "/";
        ss << uv4->getUv_index();
    }

    return ss.str();
}

