#ifndef LSMESH_NORMAL_H
#define LSMESH_NORMAL_H

#include <string>

using namespace std;

class lsMesh_Normal
{
public:
    lsMesh_Normal();
    lsMesh_Normal( lsMesh_Normal *copy );
    virtual ~lsMesh_Normal();

    // Data
    void Set_Normal( float x, float y, float z );
    float* Get_Normal();
    void Set_Tangent( float x, float y, float z );
    float* Get_Tangent();
    void Set_Binormal( float x, float y, float z );
    float* Get_Binormal();

    // get set
    lsMesh_Normal *getNext_normal() const;
    void setNext_normal(lsMesh_Normal *value);
    int getNindex() const;
    void setNindex(int value);

    // Save
    string toString();
    string toStringTangent();

private:
    lsMesh_Normal* next_normal;
    int nindex;
    // Data
    float normal[3];
    float tangent[3];
    float binormal[3];
};

#endif // LSMESH_NORMAL_H
