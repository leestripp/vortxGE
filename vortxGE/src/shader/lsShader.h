/**
 * @file lsShader.h
 * @brief Shader program management and GLSL integration
 * 
 * lsShader encapsulates an OpenGL shader program, including:
 * - Vertex and fragment shader compilation
 * - Uniform variable management
 * - Light setup for rendering
 * - PBR material parameters
 * 
 * @section shader_kinds Shader Kinds
 * 
 * The engine supports different shader types via ShaderKind:
 * - DEBUG: Debug visualization (normals, wireframe)
 * - SKYBOX: Skybox/environment rendering
 * - SHADOWMAP: Shadow map generation
 * - LIGHT_*: Various lighting models (none, diffuse, normal, PBR)
 * 
 * @section usage Usage
 * @code
 * lsShader* shader = new lsShader(render);
 * shader->Load_Shader("light_tex_dns");  // Load shader files
 * 
 * // In render loop:
 * shader->UseShader(camera, object, pass, batch);
 * @endcode
 * 
 * @see lsShaderKinds.h for shader kind constants
 * @see lsShaderManager for loading and managing shaders
 */

#pragma once

#include <iostream>
#include <map>
#include <string>

// Render
#include "../render/lsRender.h"
#include "../render/lsOpenGL.h"

// Shader kinds
#include "lsShaderKinds.h"

// Forward declarations
class lsObject;
class lsCamera;
class lsLight;
class lsPass;
class lsBatch;

using namespace std;

/**
 * @class lsShader
 * @brief Manages an OpenGL shader program
 * 
 * Encapsulates vertex and fragment shaders, provides
 * uniform management, and handles per-frame shader
 * parameter updates.
 * 
 * @section uniforms Supported Uniforms
 * 
 * - Matrices: MVP, Projection, ModelView, Model, Normal
 * - PBR: Albedo, Roughness, Metallic, Normal, AO textures
 * - Lighting: Light count, position, direction, color, intensity
 * - Shadows: Shadow matrices and texture samplers
 */
class lsShader
{
public:
    /**
     * @brief Construct a shader
     * @param render Pointer to renderer
     */
	lsShader( lsRender *render );
	
    /**
     * @brief Destructor
     * 
     * Cleans up shader program and compiled shaders.
     */
	virtual ~lsShader();
	
    /**
     * @brief Activate this shader for rendering
     * 
     * Binds the shader program and sets all uniform
     * values for the current object/camera/pass.
     * 
     * @param cam Active camera
     * @param object Object being rendered
     * @param pass Current render pass
     * @param batch Batch being rendered
     */
	void UseShader( lsCamera *cam, lsObject *object, lsPass *pass, lsBatch *batch );
	
    /**
     * @brief Load shader from files
     * 
     * Looks for .vert and .frag files with the given name
     * in the shaders directory.
     * 
     * @param name Shader base name (e.g., "light_tex_dns")
     */
	void Load_Shader( const string& name );
	
    /**
     * @brief Debug a shader object
     * @param s Shader handle to inspect
     */
	void debug( GLuint s );
	
    /**
     * @brief Check shader program for errors
     * @return true if program is valid
     */
	bool debugProgram();
	
    /**
     * @brief Set shader kind
     * @param val Shader type (DEBUG, SKYBOX, LIGHT_*, etc.)
     */
	void setKind( ShaderKind val );
	
    /**
     * @brief Get shader kind
     * @return Current shader type
     */
	ShaderKind getKind() const { return kind; }
	
	//=========================================================================
	// Lighting
	//=========================================================================
	
    /**
     * @brief Setup lights for rendering
     * 
     * Iterates through scene lights and uploads their
     * data to shader uniforms.
     * 
     * @param camera Active camera
     * @param object Object being rendered
     */
	void Setup_Lights( lsCamera *camera, lsObject *object );
	
    /**
     * @brief Set a single light uniform
     * 
     * Uploads light data to shader at given index.
     * 
     * @param index Light slot (0-2)
     * @param camera Active camera
     * @param light Light source
     * @param object Object being rendered
     */
	void Set_Light( int index, lsCamera *camera, lsLight *light, lsObject *object );
	
	//=========================================================================
	// Accessors
	//=========================================================================
	
    /**
     * @brief Get shader name
     * @return Name string
     */
	const string& getName() { return name; }
	
    /**
     * @brief Set shader name
     * @param val New name
     */
	void setName( const string& val ) { this->name = val; }
	
    /**
     * @brief Get OpenGL shader program
     * @return GLuint program handle
     */
	GLuint getProgram() const { return shader; }
	
    /**
     * @brief Get renderer
     * @return Pointer to lsRender
     */
	lsRender* getRender() const { return m_render; }

	//=========================================================================
    // Public Data Members
    //=========================================================================
	
    /** @brief Renderer reference */
	lsRender *m_render;
	
private:
    //=========================================================================
	// Private Members
	//=========================================================================
	
    /** @brief Shader name */
	string name;
	
    /** @brief Shader kind/type */
	ShaderKind kind;
	
    //=========================================================================
	// OpenGL Objects
	//=========================================================================
	
    /** @brief Compiled vertex shader */
	GLuint shader_v;
    
    /** @brief Compiled fragment shader */
	GLuint shader_f;
    
    /** @brief Linked shader program */
	GLuint shader;
	
	//=========================================================================
	// Matrix Uniforms
	//=========================================================================
	
    /** @brief Model-View-Projection matrix location */
	GLint imvpMatrix;
    
    /** @brief Projection matrix location */
	GLint ipMatrix;
    
    /** @brief Model-View matrix location */
	GLint imvMatrix;
    
    /** @brief Model matrix location */
	GLint imMatrix;
    
    /** @brief Normal matrix location */
	GLint inMatrix;
    
    /** @brief Shadow matrices (up to 3) */
	GLint isMatrices[3];
	
	//=========================================================================
	// PBR Texture Uniforms
	//=========================================================================
	
    /** @brief Albedo texture location */
	GLint iTex_albedo;
    
    /** @brief Roughness/Metallic texture location */
	GLint iTex_roughness_metallic;
    
    /** @brief Normal map texture location */
	GLint iTex_normal;
	
	//=========================================================================
	// Shadow Uniforms
	//=========================================================================
	
    /** @brief Shadow map textures (up to 3) */
	GLint iTex_shadowmaps[3];
    
    /** @brief Shadow map usage flags (up to 3) */
	GLint iUse_shadowmap[3];
	
	//=========================================================================
	// Material Uniforms
	//=========================================================================
	
    /** @brief Material albedo color */
	GLint iMatAlbedo;
    
    /** @brief Material roughness */
	GLint iMatRoughness;
    
    /** @brief Material metallic */
	GLint iMatMetallic;
	
	//=========================================================================
	// Light Uniforms
	//=========================================================================
	
    /** @brief Number of active lights */
	GLint iLightCount;
    
    /** @brief Light types (up to 3) */
	GLint iLightType[3];
    
    /** @brief Light positions (up to 3) */
	GLint iLightPos[3];
    
    /** @brief Light directions (up to 3) */
	GLint iLightDir[3];
    
    /** @brief Light inner cutoffs (spotlights) */
	GLint iLightCutoff[3];
    
    /** @brief Light outer cutoffs (spotlights) */
	GLint iLightOuterCutoff[3];
    
    /** @brief Light diffuse colors (up to 3) */
	GLint iLightDiffuse[3];
    
    /** @brief Light specular colors (up to 3) */
	GLint iLightSpecular[3];
    
    /** @brief Light intensities (up to 3) */
	GLint iLightIntensity[3];
};
