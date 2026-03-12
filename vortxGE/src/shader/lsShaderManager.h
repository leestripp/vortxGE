/**
 * @file lsShaderManager.h
 * @brief Shader program manager for loading and caching shaders
 * 
 * lsShaderManager handles loading, compilation, and caching of
 * shader programs. It provides:
 * - Shader loading from files
 * - Name-based shader lookup
 * - Shader kind mapping
 * 
 * @section usage Usage
 * @code
 * lsShaderManager* sm = new lsShaderManager(render);
 * sm->Init();  // Load default shaders
 * 
 * lsShader* shader = sm->Find_Shader("light_tex_dns");
 * @endcode
 * 
 * @see lsShader for individual shader management
 * @see lsShaderKinds.h for shader kind constants
 */

#pragma once

#include <iostream>
#include <map>
#include <string>

using namespace std;

// Shader kinds
#include "lsShaderKinds.h"

// Forward declarations
class lsShader;
class lsRender;

/**
 * @class lsShaderManager
 * @brief Manages all shader programs in the engine
 * 
 * Acts as a central registry for shader programs.
 * Provides loading and lookup functionality.
 * 
 * @section default_shaders Default Shaders
 * 
 * The manager loads these shader types:
 * - DEBUG: Debug visualization
 * - SKYBOX: Skybox rendering
 * - SHADOWMAP: Shadow generation
 * - LIGHT_*: Various lighting models
 */
class lsShaderManager
{
public:
    /**
     * @brief Construct shader manager
     * @param render Pointer to renderer
     */
    lsShaderManager( lsRender *render );
    
    /**
     * @brief Destructor
     */
    virtual ~lsShaderManager();
    
    /**
     * @brief Initialize and load default shaders
     * 
     * Loads all standard shader programs.
     */
    void Init();
    
    /**
     * @brief Add shader to manager
     * @param shader Pointer to shader to add
     */
    void Add_Shader( lsShader* shader );
    
    /**
     * @brief Remove shader from manager
     * @param shader Pointer to shader to remove
     */
    void Del_Shader( lsShader* shader );
	
    /**
     * @brief Find shader by name
     * 
     * Looks up shader by its name (without path/extension).
     * 
     * @param name Shader base name
     * @return Pointer to shader, or nullptr if not found
     */
    lsShader* Find_Shader( const string& name );
    
    /**
     * @brief Get shader kind from name
     * 
     * Maps a shader filename to its kind constant.
     * 
     * @param name Shader filename
     * @return ShaderKind constant
     */
    ShaderKind getShaderKindByName(const string& name) const;
	
    //=========================================================================
	// Legacy (kept for compatibility)
	//=========================================================================
	
    /**
     * @brief Get shader list head
     * @return nullptr (legacy, not used)
     */
    lsShader* getShaderList() const { return nullptr; }
	
private:
    //=========================================================================
	// Private Members
	//=========================================================================
	
    /** @brief Map of shader names to shader objects */
    map<string, lsShader*> shader_map;
    
    /** @brief Map of shader names to their kinds */
    map<string, ShaderKind> shaderNameToKindMap;
    
    /** @brief Renderer reference */
    lsRender *m_render;
};
