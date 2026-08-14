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

// Shader kinds
#include "lsShaderKinds.h"

// Forward declarations
class lsObject;
class lsCamera;
class lsLight;
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
	void UseShader( lsCamera *cam, lsObject *object, int pass_type, lsBatch *batch );
	
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
	void debug( lsShaderID s );
	
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
     * @return Renderer program handle (lsProgramID = uint32_t)
     */
	lsProgramID getProgram() const { return shader; }
	
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
	lsShaderID shader_v;
    
    /** @brief Compiled fragment shader */
	lsShaderID shader_f;
    
    /** @brief Linked shader program */
	lsProgramID shader;
	
	//=========================================================================
	// Matrix Uniforms
	//=========================================================================
	
    /** @brief Model-View-Projection matrix location */
	lsUniformID imvpMatrix;
    
    /** @brief Projection matrix location */
	lsUniformID ipMatrix;
    
    /** @brief Model-View matrix location */
	lsUniformID imvMatrix;
    
    /** @brief Model matrix location */
	lsUniformID imMatrix;
    
    /** @brief Normal matrix location */
	lsUniformID inMatrix;
    
    /** @brief Shadow matrices (up to 3) */
	lsUniformID isMatrices[3];
	
	//=========================================================================
	// PBR Texture Uniforms
	//=========================================================================
	
    /** @brief Albedo texture location */
	lsUniformID iTex_albedo;
    
    /** @brief Roughness/Metallic texture location */
	lsUniformID iTex_roughness_metallic;
    
    /** @brief Normal map texture location */
	lsUniformID iTex_normal;
	
	//=========================================================================
	// Shadow Uniforms
	//=========================================================================
	
    /** @brief Shadow map textures (up to 3) */
	lsUniformID iTex_shadowmaps[3];
    
    /** @brief Shadow map usage flags (up to 3): 0=off 1=single map 2=CSM */
	lsUniformID iUse_shadowmap[3];

	// L4.1 PCSS: raw-depth views + per-slot light size + preset gate
	lsUniformID iTex_shadowdepth[3];
	lsUniformID iShadowSoftness[3];
	lsUniformID iPcssEnabled;

	// L4.2: screen-space contact shadows (prepass depth ray march)
	lsUniformID iTex_prepassDepth;
	lsUniformID iContactEnabled;
	lsUniformID iContactNear;
	lsUniformID iContactFar;
	lsUniformID iPrepassFlipY;

	//=========================================================================
	// CSM Uniforms (L2.4 — used when a light slot is use_shadowmap=2)
	//=========================================================================

    /** @brief CSM depth atlas texture */
	lsUniformID iTex_csm;

    /** @brief Per-cascade shadow matrices (up to 4) */
	lsUniformID iCsmMatrices[4];

    /** @brief Per-cascade far split depths (view space) */
	lsUniformID iCsmSplits[4];

    /** @brief Active cascade count (0 = CSM off) */
	lsUniformID iCsmCount;

	//=========================================================================
	// Point shadow uniforms (L3.2 — used when a light slot is use_shadowmap=3)
	//=========================================================================

    /** @brief Point shadow cube texture (shared unit, one point caster at a time) */
	lsUniformID iTex_pointshadow;

    /** @brief Point light world positions (cube sampling needs world dir) */
	lsUniformID iPointWorldPos[3];

    /** @brief Per-slot point shadow near/far (reference depth reconstruction) */
	lsUniformID iPointNear[3];
	lsUniformID iPointFar[3];
	
	//=========================================================================
	// Material Uniforms
	//=========================================================================
	
    /** @brief Material albedo color */
	lsUniformID iMatAlbedo;
    
    /** @brief Material roughness */
	lsUniformID iMatRoughness;
    
    /** @brief Material metallic */
	lsUniformID iMatMetallic;
	
	//=========================================================================
	// Light Uniforms
	//=========================================================================
	
    /** @brief Number of active lights */
	lsUniformID iLightCount;
	lsUniformID iDebugView;		///< D1.2: vxDebugView selector (light_tex_* shaders)
    
    /** @brief Light types (up to 3) */
	lsUniformID iLightType[128]; // L5.2: raised from 3
    
    /** @brief Light positions (up to 3) */
	lsUniformID iLightPos[128]; // L5.2: raised from 3
    
    /** @brief Light directions (up to 3) */
	lsUniformID iLightDir[128]; // L5.2: raised from 3
    
    /** @brief Light inner cutoffs (spotlights) */
	lsUniformID iLightCutoff[128]; // L5.2: raised from 3
    
    /** @brief Light outer cutoffs (spotlights) */
	lsUniformID iLightOuterCutoff[128]; // L5.2: raised from 3
    
    /** @brief Light diffuse colors (up to 3) */
	lsUniformID iLightDiffuse[128]; // L5.2: raised from 3
    
    /** @brief Light specular colors (up to 3) */
	lsUniformID iLightSpecular[128]; // L5.2: raised from 3
    
    /** @brief Light intensities (up to 3) */
	lsUniformID iLightIntensity[128]; // L5.2: raised from 3
	lsUniformID iLightRadius[128]; // L5: cluster window radius

	// L5 clustered lighting (all -1 on non-light programs)
	lsUniformID iClusterEnabled = -1;
	lsUniformID iClusterGrid = -1;
	lsUniformID iClusterIndices = -1;
	lsUniformID iClusterTilesX = -1;
	lsUniformID iClusterTilesY = -1;
	lsUniformID iClusterSlices = -1;
	lsUniformID iClusterNear = -1;
	lsUniformID iClusterFar = -1;
	lsUniformID iClusterTileSize = -1;
	lsUniformID iClusterFlipY = -1;
};
