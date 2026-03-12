#ifndef LSMESH_UV_H
#define LSMESH_UV_H

#include <string>

using namespace std;

class lsMesh_UV
{
public:
    lsMesh_UV();
    virtual ~lsMesh_UV();

    // Data
    void Set_UV( float u, float v );
    float *Get_UV();

    // get set
    lsMesh_UV *getNext_uv() const;
    void setNext_uv(lsMesh_UV *value);
    int getUv_index() const;
    void setUv_index(int value);

    // Save
    string toString();

private:
    lsMesh_UV *next_uv;
    int uv_index;

    // data
    float uv[2];
};

#endif // LSMESH_UV_H
