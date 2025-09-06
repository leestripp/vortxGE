#ifndef LSMESH_VERTEX_H
#define LSMESH_VERTEX_H

#include <string>

using namespace std;

class lsMesh_Vertex
{
public:
    lsMesh_Vertex();
    virtual ~lsMesh_Vertex();

    // get and set
    lsMesh_Vertex* getNext_vertex();
    void setNext_vertex( lsMesh_Vertex* v );
    int getIndex();
    void setIndex( int i );
    float* getVertexData();
    void setVertexData( float x, float y, float z );

    // Save
    string toString();

private:
    lsMesh_Vertex* next_vertex;
    int vindex;

    // vertex
    float vertex[3];
};

#endif // LSMESH_VERTEX_H
