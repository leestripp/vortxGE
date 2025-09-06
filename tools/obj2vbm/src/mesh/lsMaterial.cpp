#include <iostream>
#include <sstream>

#include "lsMaterial.h"

lsMaterial::lsMaterial()
{
    next_material = NULL;
    name = "Not Set";

    // Default Material
    mdiffuse[0] = 0.5f;
    mdiffuse[1] = 0.5f;
    mdiffuse[2] = 0.5f;
    mdiffuse[3] = 1.0f;

    mambiant[0] = 0.2f;
    mambiant[1] = 0.2f;
    mambiant[2] = 0.2f;
    mambiant[3] = 1.0f;

    mspec[0] = 1.0f;
    mspec[1] = 1.0f;
    mspec[2] = 1.0f;
    mspec[3] = 1.0f;
    mshininess = 96.0f;
    // overall
    transparent = 1.0;
    mglow = 1.0;

    tex_d = "";
    tex_n = "";
    tex_s = "";
    shader = "";

}

lsMaterial::~lsMaterial()
{
    if( next_material ) delete next_material;
}


void lsMaterial::setDiffuse( float arg1, float arg2, float arg3 )
{
    mdiffuse[0] = arg1;
    mdiffuse[1] = arg2;
    mdiffuse[2] = arg3;
    mdiffuse[3] = 1.0f;
}

void lsMaterial::setSpecular( float arg1, float arg2, float arg3 )
{
    mspec[0] = arg1;
    mspec[1] = arg2;
    mspec[2] = arg3;
    mspec[3] = 1.0f;
}

void lsMaterial::setAmbient( float arg1, float arg2, float arg3 )
{
    mambiant[0] = arg1;
    mambiant[1] = arg2;
    mambiant[2] = arg3;
    mambiant[3] = 1.0f;
}

void lsMaterial::setShininess( float arg1 )
{
    mshininess = arg1;
}

void lsMaterial::setGlow( float arg1 )
{
    mglow = arg1;
}

float lsMaterial::getTransparent() const
{
    return transparent;
}

void lsMaterial::setTransparent(float value)
{
    transparent = value;
}

string lsMaterial::toStringDiffuse()
{
    stringstream ss;
    ss << mdiffuse[0] << " " << mdiffuse[1] << " " << mdiffuse[2] << " " << mdiffuse[3];
    return ss.str();
}

string lsMaterial::toStringAmbiant()
{
    stringstream ss;
    ss << mambiant[0] << " " << mambiant[1] << " " << mambiant[2] << " " << mambiant[3];
    return ss.str();
}

string lsMaterial::toStringSpecular()
{
    stringstream ss;
    ss << mspec[0] << " " << mspec[1] << " " << mspec[2] << " " << mspec[3];
    return ss.str();
}

float lsMaterial::getMglow() const
{
    return mglow;
}

void lsMaterial::setMglow(float value)
{
    mglow = value;
}

float lsMaterial::getMshininess() const
{
    return mshininess;
}

void lsMaterial::setMshininess(float value)
{
    mshininess = value;
}

string lsMaterial::getName() const
{
    return name;
}

void lsMaterial::setName(const string &value)
{
    name = value;
}

string lsMaterial::getTex_d() const
{
    return tex_d;
}

void lsMaterial::setTex_d(const string &value)
{
    tex_d = value;
}

string lsMaterial::getTex_n() const
{
    return tex_n;
}

void lsMaterial::setTex_n(const string &value)
{
    tex_n = value;
}

string lsMaterial::getTex_s() const
{
    return tex_s;
}

void lsMaterial::setTex_s(const string &value)
{
    tex_s = value;
}

string lsMaterial::getShader() const
{
    return shader;
}

void lsMaterial::setShader(const string &value)
{
    shader = value;
}

lsMaterial *lsMaterial::getNext_material() const
{
    return next_material;
}

void lsMaterial::setNext_material(lsMaterial *value)
{
    next_material = value;
}

