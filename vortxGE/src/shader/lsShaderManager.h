#pragma once

#include <iostream>
#include <map>
#include <string>

// Shader kings.
#include "lsShaderKinds.h"

class lsShader;
class lsRender;

using namespace std;

/**
 Manage global shaders
 @param class
 @returns none
 */
class lsShaderManager
{
public:
    lsShaderManager( lsRender *render );
    virtual ~lsShaderManager();
    
    void Init();
    void Add_Shader( lsShader* shader );
    void Del_Shader( lsShader* shader );
	
    lsShader* Find_Shader( const string& name );
    lsShader* getShaderList() const { return nullptr; } // Not used anymore
	
    // New method to map shader names to kinds
    ShaderKind getShaderKindByName(const string& name) const;
	
private:
    // Changed from linked list to map for fast lookup
    std::map<std::string, lsShader*> shader_map;
    // Map to store shader name -> kind mapping
    std::map<std::string, ShaderKind> shaderNameToKindMap;
    
    // render
    lsRender *m_render;
};
