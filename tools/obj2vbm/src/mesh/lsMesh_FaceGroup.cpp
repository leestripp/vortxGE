#include "lsMesh_FaceGroup.h"
#include "lsMesh_Face.h"

#include "lsMaterial.h"


lsMesh_FaceGroup::lsMesh_FaceGroup()
{
    next_facegroup = NULL;

    face_list = NULL;
    material = NULL;

    triangle_count = 0;
    quad_count = 0;
    smooth_shade = false;

}

lsMesh_FaceGroup::~lsMesh_FaceGroup()
{
    // DO NOT delete material

    if ( face_list ) delete face_list;
    if ( next_facegroup ) delete next_facegroup;
}

void lsMesh_FaceGroup::Add_Face( lsMesh_Face* face )
{
    face->setNext_face( face_list );
    face_list = face;

    if( face->getFace_type() == LSFT_QUAD )
    {
        face->setIndex( quad_count+1 ); // index starts at 1
        quad_count++;
    } else
    {
        face->setIndex( triangle_count+1 ); // index starts at 1
        triangle_count++;
    }
}

void lsMesh_FaceGroup::setMaterial( lsMaterial* mat )
{
    material = mat;
}

lsMaterial* lsMesh_FaceGroup::getMaterial()
{
    return material;
}

void lsMesh_FaceGroup::setNext_group( lsMesh_FaceGroup* g )
{
    next_facegroup = g;
}

lsMesh_FaceGroup* lsMesh_FaceGroup::getNext_group()
{
    return next_facegroup;
}

lsMesh_Face* lsMesh_FaceGroup::getFace_list()
{
    return face_list;
}

void lsMesh_FaceGroup::setFace_list( lsMesh_Face* face )
{
    face_list = face;
}


int lsMesh_FaceGroup::getTriangle_count()
{
    return triangle_count;
}

int lsMesh_FaceGroup::getQuad_count()
{
    return quad_count;
}

bool lsMesh_FaceGroup::getSmooth_shade() const
{
    return smooth_shade;
}

void lsMesh_FaceGroup::setSmooth_shade(bool value)
{
    smooth_shade = value;
}


