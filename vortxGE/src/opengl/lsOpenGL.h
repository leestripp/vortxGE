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

// openGL linux
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glxext.h>

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
    // bool findExtension( const string& str );

    void CheckGLErrors();

    // Store FBO
    void StoreFBO();
    void RestoreFBO();

    // Extensions
    void LoadExtensions();
    // FBOs
    PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
    PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
    PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
    PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
    PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
    // Buffers
    PFNGLGENBUFFERSPROC glGenBuffers;
    PFNGLBINDBUFFERPROC glBindBuffer;
    PFNGLDELETEBUFFERSPROC glDeleteBuffers;
    PFNGLBUFFERDATAPROC glBufferData;
    // Attrs
    PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
    PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
    PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
    PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
    // textures
    PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
    // Queries
    PFNGLGENQUERIESPROC glGenQueries;
    PFNGLBEGINQUERYPROC glBeginQuery;
    PFNGLENDQUERYPROC glEndQuery;
    PFNGLGETQUERYOBJECTIVPROC glGetQueryObjectiv;
    PFNGLDELETEQUERIESPROC glDeleteQueries;
    // Shaders
    PFNGLUSEPROGRAMPROC glUseProgram;
    PFNGLDELETEPROGRAMPROC glDeleteProgram;
    PFNGLCREATESHADERPROC glCreateShader;
    PFNGLDETACHSHADERPROC glDetachShader;
    PFNGLDELETESHADERPROC glDeleteShader;
    PFNGLCREATEPROGRAMPROC glCreateProgram;
    PFNGLATTACHSHADERPROC glAttachShader;
    PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
    PFNGLSHADERSOURCEPROC glShaderSource;
    PFNGLCOMPILESHADERPROC glCompileShader;
    PFNGLLINKPROGRAMPROC glLinkProgram;
    PFNGLGETSHADERIVPROC glGetShaderiv;
    PFNGLGETPROGRAMIVPROC glGetProgramiv;
    PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
    PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;

    // Uniforms
    PFNGLUNIFORM1IPROC glUniform1i;
    PFNGLUNIFORM4FPROC glUniform4f;
    PFNGLUNIFORM1FPROC glUniform1f;
    PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
    PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
    PFNGLUNIFORM3FPROC glUniform3f;
    PFNGLUNIFORM2FPROC glUniform2f;


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
