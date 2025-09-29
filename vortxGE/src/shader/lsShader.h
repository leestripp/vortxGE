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
const int LSST_LIGHT_TEX_DNS			= 4;
const int LSST_OCCLUSION            = 5;
const int LSST_SHADELESS_TEX_D		= 6;
const int LSST_SHADELESS_TEX_NONE	= 7;
const int LSST_COMP                 = 8;
const int LSST_QUAD                 = 9;
const int LSST_LIGHT_TEX_NONE		= 10;
const int LSST_BLOOM                = 11;
const int LSST_DEBUG_NORMALS        = 12;
const int LSST_SHADOWMAP            = 13;
const int LSST_SKYBOX               = 14; // New skybox shader type

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
	// Changed to array to match shader uniform arrays
	void Set_Light( int index, lsCamera *camera, lsLight *light, lsObject *object );
	
	
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
		this->prev_shader = val;
	}

	// Data ---------------------------

	const string& getName()
	{
		return name;
	}

	void setName( const string& val )
	{
		this->name = val;
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
	GLint isMatrices[3];    // Array for up to 3 Shadow Matrices
	
	// PBR Textures
	GLint iTex_albedo;
	GLint iTex_roughness_metallic;
	GLint iTex_normal;

	// Shadowmap uniform arrays
	GLint iTex_shadowmaps[3]; // Array for up to 3 Shadowmap textures
	GLint iUse_shadowmap[3];  // Array for up to 3 use_shadowmap flags
	
	// Material
	GLint iMatAlbedo;
	GLint iMatRoughness;
	GLint iMatMetallic;
	
	// Light locations (arrays for up to 3 lights)
	GLint iLightCount;
	GLint iLightType[3];
	GLint iLightPos[3];
	GLint iLightDir[3];
	GLint iLightCutoff[3];
	GLint iLightOuterCutoff[3];
	GLint iLightDiffuse[3];
	GLint iLightSpecular[3];
	GLint iLightIntensity[3];
};
