#ifndef LSMESH_FACEGROUP_H
#define LSMESH_FACEGROUP_H

class lsMesh_Face;
class lsMaterial;
class lsShader_Program;


class lsMesh_FaceGroup
{
public:
    lsMesh_FaceGroup();
    virtual ~lsMesh_FaceGroup();

    void Add_Face( lsMesh_Face* face );

    void setMaterial( lsMaterial* mat );
    class lsMaterial* getMaterial();

    // Get set
    void setNext_group( lsMesh_FaceGroup* g );
    lsMesh_FaceGroup* getNext_group();
    class lsMesh_Face* getFace_list();
    void setFace_list( class lsMesh_Face* face );
    bool getSmooth_shade() const;
    void setSmooth_shade(bool value);
    int getTriangle_count();
    int getQuad_count();

    // Save

private:
    lsMesh_FaceGroup* next_facegroup;

    lsMesh_Face* face_list;

    class lsMaterial* material;
    int  triangle_count;
    int  quad_count;
    bool smooth_shade;

};

#endif // LSMESH_FACEGROUP_H
