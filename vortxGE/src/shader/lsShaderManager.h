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

#ifndef vortxGE_lsShaderManager_h
#define vortxGE_lsShaderManager_h

#include <iostream>

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

    lsShader* getShaderList() const
    {
        return shader_list;
    }

private:
    lsShader *shader_list;

};


#endif
