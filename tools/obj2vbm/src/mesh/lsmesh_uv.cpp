#include <iostream>
#include <sstream>

#include "lsmesh_uv.h"

lsMesh_UV::lsMesh_UV()
{
    next_uv = NULL;
    uv_index = 0;
}

lsMesh_UV::~lsMesh_UV()
{
    if( next_uv ) delete next_uv;
}

void lsMesh_UV::Set_UV(float u, float v)
{
    uv[0] = u;
    uv[1] = v;
}

float *lsMesh_UV::Get_UV()
{
    return uv;
}

lsMesh_UV *lsMesh_UV::getNext_uv() const
{
    return next_uv;
}

void lsMesh_UV::setNext_uv(lsMesh_UV *value)
{
    next_uv = value;
}

int lsMesh_UV::getUv_index() const
{
    return uv_index;
}

void lsMesh_UV::setUv_index(int value)
{
    uv_index = value;
}

string lsMesh_UV::toString()
{
    stringstream ss;
    ss << uv_index << " " << uv[0] << " " << uv[1];
    return ss.str();
}

