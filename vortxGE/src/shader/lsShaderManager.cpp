// core
#include "../vortx_debug.h"

// opengl
#include "../opengl/lsOpenGL.h"

#include "lsShaderManager.h"
#include "lsShader.h"


lsShaderManager::lsShaderManager( lsOpenGL *opengl )
{
    ogl = opengl;

    shader_list = NULL;
}

lsShaderManager::~lsShaderManager()
{
    while( shader_list )
    {
        Del_Shader(shader_list);
    }
}

void lsShaderManager::Init()
{
#ifdef VORTX_DEBUG_SHADERS
    cout << "lsShaderManager::Init()" << endl;
#endif

    // load built-in shaders
    lsShader *shd;

    // Default Shadeless
    shd = new lsShader( ogl );
    if (shd)
    {
        shd->Load_Shader("debug");
        Add_Shader( shd );
    }

    shd = new lsShader( ogl );
    if (shd)
    {
        shd->Load_Shader("quad");
        Add_Shader( shd );
    }
}

void lsShaderManager::Add_Shader( lsShader* shader )
{
#ifdef VORTX_DEBUG_SHADERS
    cout << "lsShaderManager::Add_Shader() - Name: " << shader->getName() << endl;
#endif

    shader->setNext_shader( shader_list );
    if( shader_list ) shader_list->setPrev_shader( shader );
    shader_list = shader;
}

void lsShaderManager::Del_Shader( lsShader* shader )
{
#ifdef VORTX_DEBUG_SHADERS
    cout << "sShaderManager::Del_Shader() - Name: " << shader->getName() << endl;
#endif

    if( shader->getPrev_shader() == NULL )
    {
        // first in list
        shader_list = shader->getNext_shader();
        if( shader_list ) shader_list->setPrev_shader( NULL );
    } else
    {
        // Close the gap
        shader->getPrev_shader()->setNext_shader( shader->getNext_shader() );
        if( shader->getNext_shader() )
        {
            shader->getNext_shader()->setPrev_shader( shader->getPrev_shader() );
        }
    }
    delete shader;
}

lsShader* lsShaderManager::Find_Shader( const string &name )
{
#ifdef VORTX_DEBUG_SHADERS
    cout << "lsShaderManager::Find_Shader() - Name: " << name << endl;
#endif

    lsShader* shader;

    shader = shader_list;
    while( shader )
    {
        if( shader->getName() == name ) break;
        shader = shader->getNext_shader();
    }

#ifdef VORTX_DEBUG_SHADERS
    if( shader )
      cout << "lsShaderManager::Find_Shader() - Shader Found: " << shader->getName() << endl;
#endif

    return shader;
}

