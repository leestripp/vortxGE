/*
 * Copyright (c) 2016 by Lee A. Stripp <leestripp@gmail.com>
 *
 * This file is part of the vortxGE library.
 *
 * vortxGE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * vortxGE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with vortxGE.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef vortxGE_lsShader_h
#define vortxGE_lsShader_h

#include <iostream>

// OpenGL
#include "../opengl/lsOpenGL.h"

class lsObject;
class lsCamera;
class lsLight;
class lsPass;
class lsBatch;

// Shader types
const int LSST_NONE                 = 0;
const int LSST_DEBUG                = 1;
const int LSST_LIGHT_TEX_D          = 2;
const int LSST_LIGHT_TEX_DN         = 3;
const int LSST_LIGHT_TEX_DNS		= 4;
const int LSST_OCCLUSION            = 5;
const int LSST_SHADELESS_TEX_D		= 6;
const int LSST_SHADELESS_TEX_NONE	= 7;
const int LSST_COMP                 = 8;
const int LSST_QUAD                 = 9;
const int LSST_LIGHT_TEX_NONE		= 10;
const int LSST_BLOOM                = 11;
const int LSST_DEBUG_NORMALS        = 12;

using namespace std;

/**
 Holds a openGL shader program
 @param class
 @returns none
 */
class lsShader
{
public:
    lsShader( lsOpenGL *opengl );
    virtual ~lsShader();

    // opengl
    lsOpenGL *ogl;

    void UseShader( lsCamera *cam, lsObject *object, lsPass *pass, lsBatch *batch );
    void Load_Shader( const string& name );
    void debug( GLuint s );
    bool debugProgram();
    void setShader_type( int val );

    // Lights
    void Setup_Lights( lsCamera *camera, lsObject *object );
    void Set_Light( int i, lsCamera *camera, lsLight *light, lsObject *object );


    // Navi ---------------------------

    lsShader* getNext_shader() const
    {
        return next_shader;
    }

    void setNext_shader( lsShader* val )
    {
        next_shader = val;
    }

    lsShader* getPrev_shader() const
    {
        return prev_shader;
    }

    void setPrev_shader( lsShader* val )
    {
        prev_shader = val;
    }

    // Data ---------------------------

    const string& getName()
    {
        return name;
    }

    void setName( const string& val )
    {
        name = val;
    }

    int getShader_type() const
    {
        return shader_type;
    }

    GLuint getProgram() const
    {
        return shader;
    }

private:
    lsShader *next_shader;
    lsShader *prev_shader;
    string name;
    int shader_type;

    // Shader names
    GLuint shader_v;
    GLuint shader_f;
    GLuint shader;

	// uniforms
	GLint imvpMatrix;
    GLint ipMatrix;         // Projection Matrix
    GLint imvMatrix;        // Model View Matrix
    GLint inMatrix;         // Normal Matrix
    GLint isMatrix;         // Shadow Matrix
	// texture locations
	GLint iTex_diffuse;
	GLint iTex_normal;
	GLint iTex_specular;
    // Shadowmap locations
	GLint iTex_shadowmap;
	GLint iShadow_bias;
	GLint iPixel_x;
	GLint iPixel_y;
	GLint iUse_shadowmap;
	// Material
	GLint iMat_diffuse;
	GLint iMat_specular;
	GLint iMat_ambiant;
	GLint iMat_shininess;
	GLint iGlow;
	// Light locations
    GLint iLightCount;
    // Light 1
	GLint iLightType1;
    GLint iLightPos1;
    GLint iLightDiffuse1;
    GLint iLightSpecular1;
    // light 2
    GLint iLightType2;
    GLint iLightPos2;
    GLint iLightDiffuse2;
    GLint iLightSpecular2;
    // light 3
    GLint iLightType3;
    GLint iLightPos3;
    GLint iLightDiffuse3;
    GLint iLightSpecular3;

    // Materail
    GLuint iMatAmbiant;
    GLuint iMatDiffuse;
    GLuint iMatSpecular;
    GLuint iMatShininess;
    GLuint iMatGlow;

};


#endif
