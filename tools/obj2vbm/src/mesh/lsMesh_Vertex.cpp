#include <sstream>

#include "lsMesh_Vertex.h"


lsMesh_Vertex::lsMesh_Vertex()
{
    next_vertex = 0;
    vindex = 0;

    // data
    vertex[0] = 0;
    vertex[1] = 0;
    vertex[2] = 0;
}

lsMesh_Vertex::~lsMesh_Vertex()
{
    if ( next_vertex ) delete next_vertex;
}


lsMesh_Vertex* lsMesh_Vertex::getNext_vertex()
{
    return next_vertex;
}

void lsMesh_Vertex::setNext_vertex( lsMesh_Vertex* v )
{
    next_vertex = v;
}

int lsMesh_Vertex::getIndex()
{
    return vindex;
}

void lsMesh_Vertex::setIndex( int i )
{
    vindex = i;
}

float* lsMesh_Vertex::getVertexData()
{
    return vertex;
}

void lsMesh_Vertex::setVertexData( float x, float y, float z )
{
    vertex[0] = x;
    vertex[1] = y;
    vertex[2] = z;
}

string lsMesh_Vertex::toString()
{
    stringstream ss;
    ss << vindex << " " << vertex[0] << " " << vertex[1] << " " << vertex[2];
    return ss.str();
}

