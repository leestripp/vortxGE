/*
 * Copyright (c) 2016 by Lee A. Stripp <leestripp@gmail.com>
 *
 * This file is part of the vortxGE library.
 *
 * vortxGE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * vortxGE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with vortxGE.  If not, see <http://www.gnu.org/licenses/>
 */

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
    cout << "sShaderManager::Find_Shader() - Name: " << name << endl;
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
      cout << "sShaderManager::Find_Shader() - Shader Found: " << shader->getName() << endl;
#endif
    
    return shader;
}

