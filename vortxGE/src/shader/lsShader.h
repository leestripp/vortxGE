#pragma once

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
const int LSST_SHADOWMAP            = 13;

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
    GLint imMatrix;         // Model Matrix
    GLint inMatrix;         // Normal Matrix
    GLint isMatrix;         // Shadow Matrix
    
	// texture locations
	GLint iTex_diffuse;
	GLint iTex_normal;
	GLint iTex_specular;
	
	// TODO: We need one per light.
    // Shadowmap locations
	GLint iTex_shadowmap;
	GLint iShadow_bias;
	GLint iPixel_x;
	GLint iPixel_y;
	GLint iUse_shadowmap;
	
	// Material
	GLint iMat_diffuse;
	GLint iMat_specular;
	GLint iMat_shininess;
	GLint iMat_ambiant;
	GLint iGlow;
	// Light locations
    GLint iLightCount;
    
    // Light 1
	GLint iLightType1;
    GLint iLightPos1;
    GLint iLightDir1;
    GLint iLightCutoff1;
    GLint iLightOuterCutoff1;
    GLint iLightDiffuse1;
    GLint iLightSpecular1;
    GLint iLightIntensity1;
    // light 2
    GLint iLightType2;
    GLint iLightPos2;
    GLint iLightDir2;
    GLint iLightCutoff2;
    GLint iLightOuterCutoff2;
    GLint iLightDiffuse2;
    GLint iLightSpecular2;
    GLint iLightIntensity2;
    // light 3
    GLint iLightType3;
    GLint iLightPos3;
    GLint iLightDir3;
    GLint iLightCutoff3;
    GLint iLightOuterCutoff3;
    GLint iLightDiffuse3;
    GLint iLightSpecular3;
    GLint iLightIntensity3;
	
    // Materail
    GLuint iMatAmbiant;
    GLuint iMatDiffuse;
    GLuint iMatSpecular;
    GLuint iMatShininess;
    GLuint iMatGlow;

};
