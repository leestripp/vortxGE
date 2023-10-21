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

#ifndef vortxGE_lsOpenGL_h
#define vortxGE_lsOpenGL_h


#include <iostream>
#include <string>

// openGL
#include <GL/glew.h>

class lsStats;
class lsShaderManager;

using namespace std;

class lsOpenGL
{
public:
    lsOpenGL();
    virtual ~lsOpenGL();

    void Init();
    void glInfo();
    void rendererChanged();

    void CheckGLErrors( const string &name );

    // Store FBO
    void StoreFBO();
    void RestoreFBO();

    lsStats* getStats() const
    {
        return stats;
    }

    void setStats( lsStats* val )
    {
        stats = val;
    }

    lsShaderManager* getShaderManager() const
    {
        return shadermanager;
    }

    void setShaderManager( lsShaderManager* val )
    {
        shadermanager = val;
    }

    // Features ********

    bool hasVBO() const
    {
        return m_hasVBO;
    }

    bool hasFBO() const
    {
        return m_hasFBO;
    }

    bool hasShaders() const
    {
        return m_hasShaders;
    }

private:
    lsStats             *stats;
    lsShaderManager     *shadermanager;

    // Store FBO
    GLint default_FBO;
    // Features
    bool m_hasVBO;
    bool m_hasFBO;
    bool m_hasShaders;

};


#endif
