// core
#include "../vortx_debug.h"

// render
#include "../render/lsRender.h"

#include "lsShaderManager.h"
#include "lsShader.h"


lsShaderManager::lsShaderManager( lsRender *render )
{
    m_render = render;
}

lsShaderManager::~lsShaderManager()
{
    // Delete all shaders in the map
    for (auto& pair : shader_map)
    {
        delete pair.second;
    }
    shader_map.clear();
}

void lsShaderManager::Init()
{
#ifdef VORTX_DEBUG_SHADERS
    cout << "lsShaderManager::Init()" << endl;
#endif
	
    // load built-in shaders
    lsShader *shd;
	
    // Default Shadeless
    shd = new lsShader( m_render );
    if(shd)
    {
        shd->Load_Shader("debug");
        Add_Shader( shd );
    }
	
    shd = new lsShader( m_render );
    if (shd)
    {
        shd->Load_Shader("quad");
        Add_Shader( shd );
    }
	
    shd = new lsShader( m_render );
    if (shd)
    {
        shd->Load_Shader("shadowmap");
        Add_Shader( shd );
    }
}

void lsShaderManager::Add_Shader( lsShader* shader )
{
#ifdef VORTX_DEBUG_SHADERS
    cout << "lsShaderManager::Add_Shader() - Name: " << shader->getName() << endl;
#endif

    // Store in map with name as key
    shader_map[shader->getName()] = shader;
}

void lsShaderManager::Del_Shader( lsShader* shader )
{
#ifdef VORTX_DEBUG_SHADERS
    cout << "lsShaderManager::Del_Shader() - Name: " << shader->getName() << endl;
#endif

    // Remove from map
    auto it = shader_map.find(shader->getName());
    if (it != shader_map.end()) {
        shader_map.erase(it);
    }
    
    delete shader;
}

lsShader* lsShaderManager::Find_Shader( const string &name )
{
#ifdef VORTX_DEBUG_SHADERS
    cout << "lsShaderManager::Find_Shader() - Name: " << name << endl;
#endif

    auto it = shader_map.find(name);
    if (it != shader_map.end()) {
#ifdef VORTX_DEBUG_SHADERS
        cout << "lsShaderManager::Find_Shader() - Shader Found: " << it->second->getName() << endl;
#endif
        return it->second;
    }

    return nullptr;
}

ShaderKind lsShaderManager::getShaderKindByName(const string& name) const
{
    auto it = shaderNameToKindMap.find(name);
    if (it != shaderNameToKindMap.end()) {
        return it->second;
    }
    
    // Default fallback
    return ShaderKind::None;
}
