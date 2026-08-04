/**
 * @file lsOpenGL.h
 * @brief OpenGL implementation of the lsRender interface
 *
 * `lsOpenGL` is the only concrete backend implemented today. It
 * translates every call in the `lsRender` abstract API into matching
 * `gl*()` calls (or `glew*` calls). This translation unit is the
 * **only** place in the engine that includes `<GL/glew.h>`.
 *
 * @section scope Scope
 *
 * The class overrides every pure-virtual method declared on `lsRender`.
 * Each override body lives in `lsOpenGL.cpp` and is a one-line forward
 * to the matching GL function (with engine-level types like `lsEnum`
 * implicitly converted to `GLenum` etc. at the boundary).
 *
 * @section newbackend Adding a new backend
 *
 * See `lsRender.h` for the contract. A `lsVulkan` subclass would
 * override the same set of methods and translate the engine-level
 * parameters into Vulkan calls — the rest of the engine does not
 * change.
 *
 * @see lsRender for the abstract interface.
 * @see lsGLTypes.h for the engine-level parameter types.
 */

#pragma once

#include <iostream>
#include <string>

// openGL — included only here and in lsOpenGL.cpp.
#include <GL/glew.h>

// Base class — note that lsRender.h itself does NOT include <GL/glew.h>.
#include "lsRender.h"

class lsStats;
class lsShaderManager;

using namespace std;

/**
 * @class lsOpenGL
 * @brief Concrete OpenGL backend.
 *
 * Implements the full `lsRender` API using OpenGL 4.3+ via GLEW.
 *
 * @section lifecycle Lifecycle
 *
 * 1. `new lsOpenGL`
 * 2. The host creates an OpenGL context.
 * 3. `engine->Init()` calls `render->Init()` which calls `lsOpenGL::Init()`.
 * 4. The host drives the engine's update/render loop; subsystems call
 *    `render->Method(...)` which dispatches to this class.
 * 5. `delete` cleans up. This destructor does NOT call any GL function
 *    (resources are owned by `lsFBO`, `lsTexture`, `lsBatch`, etc.).
 *
 * @see lsRender
 */
class lsOpenGL : public lsRender
{
public:
    lsOpenGL();
    virtual ~lsOpenGL();

    //=========================================================================
    // lsRender overrides — lifecycle
    //=========================================================================

    virtual bool Init() override;

    // Frame lifecycle shims — GL context and swap are app-owned (glfwApp),
    // so these are no-ops. Present/swap unification is deferred to the
    // V6 parity phase of the Vulkan plan.
    virtual void SetWindow( void* nativeWindow ) override;
    virtual void BeginFrame() override;
    virtual void EndFrame() override;
    virtual void Present() override;
    virtual void Resize( int width, int height ) override;
    virtual void CheckErrors( const string& name ) override;
    virtual void StoreDefaultFramebuffer() override;
    virtual void RestoreDefaultFramebuffer() override;

    //=========================================================================
    // OpenGL-specific diagnostics (printed once during Init())
    //
    // This is the only lsOpenGL-specific (non-overridden) method. It is not
    // part of the lsRender API because "print version info" is intrinsically
    // backend-specific — lsVulkan would have its own.
    //=========================================================================
    void GetInfo();

    //=========================================================================
    // lsRender overrides — texture objects
    //=========================================================================

    virtual void GenTextures( lsSizei n, lsTextureID* textures ) override;
    virtual void DeleteTextures( lsSizei n, lsTextureID* textures ) override;
    virtual void BindTexture( lsEnum target, lsTextureID texture ) override;
    virtual void ActiveTexture( lsEnum texture ) override;
    virtual void TexImage2D( lsEnum target, lsInt level, lsInt internalformat,
                             lsSizei width, lsSizei height, lsInt border,
                             lsEnum format, lsEnum type, const void* data ) override;
    virtual void TexParameteri( lsEnum target, lsEnum pname, lsInt param ) override;
    virtual void TexParameterf( lsEnum target, lsEnum pname, lsFloat param ) override;
    virtual void TexParameterfv( lsEnum target, lsEnum pname, const lsFloat* params ) override;
    virtual void GenerateMipmap( lsEnum target ) override;
    virtual void PixelStorei( lsEnum pname, lsInt param ) override;

    //=========================================================================
    // lsRender overrides — framebuffer objects
    //=========================================================================

    virtual void GenFramebuffers( lsSizei n, lsFBOID* fbos ) override;
    virtual void DeleteFramebuffers( lsSizei n, lsFBOID* fbos ) override;
    virtual void BindFramebuffer( lsEnum target, lsFBOID fbo ) override;
    virtual void FramebufferTexture2D( lsEnum target, lsEnum attachment,
                                       lsEnum textarget, lsTextureID texture,
                                       lsInt level ) override;
    virtual lsEnum CheckFramebufferStatus( lsEnum target ) override;
    virtual void DrawBuffer( lsEnum mode ) override;
    virtual void ReadBuffer( lsEnum mode ) override;
    virtual void ReadPixels( lsInt x, lsInt y, lsSizei width, lsSizei height,
                             lsEnum format, lsEnum type, void* data ) override;

    //=========================================================================
    // lsRender overrides — buffer / VAO objects
    //=========================================================================

    virtual void GenBuffers( lsSizei n, lsBufferID* buffers ) override;
    virtual void DeleteBuffers( lsSizei n, lsBufferID* buffers ) override;
    virtual void BindBuffer( lsEnum target, lsBufferID buffer ) override;
    virtual void BufferData( lsEnum target, lsSizeiptr size,
                             const void* data, lsEnum usage ) override;
    virtual void GenVertexArrays( lsSizei n, lsVAOID* arrays ) override;
    virtual void DeleteVertexArrays( lsSizei n, lsVAOID* arrays ) override;
    virtual void BindVertexArray( lsVAOID vao ) override;
    virtual void EnableVertexAttribArray( lsUInt index ) override;
    virtual void VertexAttribPointer( lsUInt index, lsInt size, lsEnum type,
                                      lsBoolean normalized, lsSizei stride,
                                      const void* pointer ) override;

    //=========================================================================
    // lsRender overrides — draw calls
    //=========================================================================

    virtual void DrawArrays( lsEnum mode, lsInt first, lsSizei count ) override;
    virtual void DrawElements( lsEnum mode, lsSizei count, lsEnum type,
                               const void* indices ) override;

    //=========================================================================
    // lsRender overrides — shader / program objects
    //=========================================================================

    virtual lsShaderID CreateShader( lsEnum type ) override;
    virtual lsProgramID CreateProgram() override;
    virtual void ShaderSource( lsShaderID shader, lsSizei count,
                               const lsChar** string, const lsInt* length ) override;
    virtual void CompileShader( lsShaderID shader ) override;
    virtual void AttachShader( lsProgramID program, lsShaderID shader ) override;
    virtual void DetachShader( lsProgramID program, lsShaderID shader ) override;
    virtual void LinkProgram( lsProgramID program ) override;
    virtual void UseProgram( lsProgramID program ) override;
    virtual void DeleteShader( lsShaderID shader ) override;
    virtual void DeleteProgram( lsProgramID program ) override;
    virtual lsUniformID GetUniformLocation( lsProgramID program, const lsChar* name ) override;
    virtual void GetShaderiv( lsShaderID shader, lsEnum pname, lsInt* params ) override;
    virtual void GetShaderInfoLog( lsShaderID shader, lsSizei bufSize,
                                   lsSizei* length, lsChar* infoLog ) override;
    virtual void GetProgramiv( lsProgramID program, lsEnum pname, lsInt* params ) override;
    virtual void GetProgramInfoLog( lsProgramID program, lsSizei bufSize,
                                    lsSizei* length, lsChar* infoLog ) override;

    //=========================================================================
    // lsRender overrides — uniform setters
    //=========================================================================

    virtual void Uniform1i( lsUniformID location, lsInt v0 ) override;
    virtual void Uniform1f( lsUniformID location, lsFloat v0 ) override;
    virtual void Uniform3f( lsUniformID location, lsFloat v0, lsFloat v1, lsFloat v2 ) override;
    virtual void Uniform4f( lsUniformID location, lsFloat v0, lsFloat v1, lsFloat v2, lsFloat v3 ) override;
    virtual void UniformMatrix3fv( lsUniformID location, lsSizei count,
                                   lsBoolean transpose, const lsFloat* value ) override;
    virtual void UniformMatrix4fv( lsUniformID location, lsSizei count,
                                   lsBoolean transpose, const lsFloat* value ) override;

    //=========================================================================
    // lsRender overrides — pipeline / rasterizer state
    //=========================================================================

    virtual void Enable( lsEnum cap ) override;
    virtual void Disable( lsEnum cap ) override;
    virtual void CullFace( lsEnum mode ) override;
    virtual void BlendFunc( lsEnum sfactor, lsEnum dfactor ) override;
    virtual void DepthFunc( lsEnum func ) override;
    virtual void DepthMask( lsBoolean flag ) override;
    virtual void DepthRangef( lsFloat near, lsFloat far ) override;
    virtual void ColorMask( lsBoolean r, lsBoolean g, lsBoolean b, lsBoolean a ) override;
    virtual void PolygonOffset( lsFloat factor, lsFloat units ) override;
    virtual void ShadeModel( lsEnum mode ) override;
    virtual void Viewport( lsInt x, lsInt y, lsSizei w, lsSizei h ) override;

    //=========================================================================
    // lsRender overrides — buffer clear
    //=========================================================================

    virtual void Clear( lsBitfield mask ) override;
    virtual void ClearColor( lsFloat r, lsFloat g, lsFloat b, lsFloat a ) override;
    virtual void ClearDepth( lsDouble d ) override;

    //=========================================================================
    // lsRender overrides — occlusion / timer queries
    //=========================================================================

    virtual void GenQueries( lsSizei n, lsQueryID* ids ) override;
    virtual void DeleteQueries( lsSizei n, lsQueryID* ids ) override;
    virtual void BeginQuery( lsEnum target, lsQueryID id ) override;
    virtual void EndQuery( lsEnum target ) override;
    virtual void GetQueryObjectiv( lsQueryID id, lsEnum pname, lsInt* params ) override;

    //=========================================================================
    // lsRender overrides — diagnostic getters
    //=========================================================================

    virtual lsEnum GetError() override;
    virtual void GetIntegerv( lsEnum pname, lsInt* params ) override;

private:
    // Framebuffer ID captured by StoreDefaultFramebuffer() for Restore...
    lsInt m_defaultFBO;
};
