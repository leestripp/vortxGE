#pragma once

#include <iostream>
#include <map>
#include <string>

// Render
#include "../render/lsRender.h"
#include "../render/lsOpenGL.h"

// Shader kings.
#include "lsShaderKinds.h"

class lsObject;
class lsCamera;
class lsLight;
class lsPass;
class lsBatch;

using namespace std;

/**
 Holds a openGL shader program
 @param class
 @returns none
 */
class lsShader
{
public:
	lsShader( lsRender *render );
	virtual ~lsShader();
	
	void UseShader( lsCamera *cam, lsObject *object, lsPass *pass, lsBatch *batch );
	void Load_Shader( const string& name );
	void debug( GLuint s );
	bool debugProgram();
	
	void setKind( ShaderKind val );
	ShaderKind getKind() const { return kind; }
	
	// Lights
	void Setup_Lights( lsCamera *camera, lsObject *object );
	void Set_Light( int index, lsCamera *camera, lsLight *light, lsObject *object );
	
	// Data ---------------------------
	const string& getName() { return name; }
	void setName( const string& val ) { this->name = val; }
	
	GLuint getProgram() const { return shader; }
	
	// render
	lsRender *m_render;
	
private:
	string name;
	ShaderKind kind;
	
	// Shader program
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
	GLint iTex_roughness_metallic;		// r = roughness, g = metallic, b = AO
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
