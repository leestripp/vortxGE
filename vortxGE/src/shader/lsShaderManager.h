#pragma once

#include <iostream>
#include <map>
#include <string>

// Shader kings.
#include "lsShaderKinds.h"

class lsShader;
class lsOpenGL;

using namespace std;

/**
 Manage global shaders
 @param class
 @returns none
 */
class lsShaderManager
{
public:
    lsShaderManager( lsOpenGL *opengl );
    virtual ~lsShaderManager();
    // opengl
    lsOpenGL *ogl;

    void Init();
    void Add_Shader( lsShader* shader );
    void Del_Shader( lsShader* shader );

    lsShader* Find_Shader( const string& name );

    lsShader* getShaderList() const { return shader_list; }

    // New method to map shader names to kinds
    ShaderKind getShaderKindByName(const string& name) const;

private:
    lsShader *shader_list;
    
    // Map to store shader name -> kind mapping
    std::map<std::string, ShaderKind> shaderNameToKindMap;
};
