/**
 * @file lsRender.h
 * @brief Abstract renderer interface — the engine's contract with any backend
 *
 * `lsRender` is the **only** renderer-facing type the rest of the engine
 * talks to. Concrete backends (`lsOpenGL`, a future `lsVulkan`, …)
 * implement every pure-virtual method below.
 *
 * @section agnosticity Backend agnosticity
 *
 * `lsRender.h` is **renderer-agnostic**: it does NOT include
 * `<GL/glew.h>`, does NOT name any concrete subclass, and uses the
 * engine-level typedefs from `lsGLTypes.h` (e.g. `lsTextureID`,
 * `lsEnum`) for every API parameter. Concrete backends may need
 * additional headers — that coupling lives inside `lsOpenGL.h/.cpp`,
 * not here.
 *
 * @section addingbackend Adding a new backend
 *
 * To add e.g. `lsVulkan`:
 *
 * 1. Derive a new class from `lsRender`.
 * 2. Override every pure-virtual method listed below.
 * 3. Implement `lsRenderTypes.h::RenderType` with the new value and
 *    update the engine entry point (`vortxGE::Init`) to instantiate it.
 *
 * Nothing in the rest of the engine needs to change.
 *
 * @see lsOpenGL for the OpenGL implementation.
 * @see lsGLTypes.h for the engine-level API types.
 * @see lsRenderTypes.h for the renderer-type enum.
 */

#pragma once

#include <iostream>
#include <string>

// Engine-level renderer-agnostic types (handles, enums, sizes).
// This header intentionally does NOT include <GL/glew.h>.
#include "lsGLTypes.h"

// Engine-level renderer constants (LS_TEXTURE_2D, LS_TRIANGLES, …).
// Currently these are #defined to GL_* via lsRenderConstants.h; a
// future non-GL backend just rewrites lsRenderConstants.h so the LS_*
// values map to the new backend's constants. Including them here
// means every file that pulls in the renderer abstraction gets the
// constants automatically — no need for <GL/glew.h> at the call site.
#include "lsRenderConstants.h"

// Renderer-type enum (OpenGL, Vulkan, ...).
#include "lsRenderTypes.h"

// Forward declarations
class lsStats;
class lsShaderManager;

using namespace std;

/**
 * @class lsRender
 * @brief Abstract base class for the engine's rendering system.
 *
 * This class defines the **complete** interface the engine uses to
 * drive rendering. Every method is virtual; most are pure-virtual
 * (`= 0`) and must be implemented by every concrete backend.
 *
 * Concrete backends:
 * - `lsOpenGL` — the only one implemented today.
 * - (Future) `lsVulkan`, `lsMetal`, …
 *
 * @section threading Threading
 *
 * `lsRender` itself is not thread-safe. Call from the main thread.
 * Backends may internally use worker threads (e.g. for shader
 * compilation) but the entry points listed below are main-thread only.
 *
 * @section lifecycle Lifecycle
 *
 * 1. `new lsConcreteBackend`
 * 2. The host application creates a valid GPU context.
 * 3. `render->Init()` — backend initialises its internal state.
 * 4. Each frame: `render->Update(...)` (if any) + draw passes.
 * 5. `delete render` — backend cleans up.
 */
class lsRender
{
public:
    /**
     * @brief Default constructor.
     *
     * Subclasses initialise their type via `setType()` in their own
     * constructor.
     */
    lsRender();

    /**
     * @brief Virtual destructor — must be virtual for proper cleanup
     * of derived classes.
     */
    virtual ~lsRender();

    //=========================================================================
    // Lifecycle
    //=========================================================================

    /**
     * @brief Initialise the renderer after a GPU context is available.
     *
     * Should set up default states, query driver info, and prepare any
     * internal caches.
     *
     * @return true on success. Backends that cannot initialise (e.g.
     * Vulkan with no capable device) return false so the caller can
     * fall back to another backend.
     */
    virtual bool Init() = 0;

    //=========================================================================
    // Frame lifecycle (Vulkan-primary additions; GL implements as shims)
    //=========================================================================

    /**
     * @brief Hand the backend its native window handle (e.g. GLFWwindow*).
     *
     * Called by the host application before Init(). GL ignores it (the
     * app owns the GL context); Vulkan needs it to create the window
     * surface.
     */
    virtual void SetWindow( void* nativeWindow ) = 0;

    /**
     * @brief Begin a frame — acquire resources, open command recording.
     * Called once per frame before any passes run. GL: no-op.
     */
    virtual void BeginFrame() = 0;

    /**
     * @brief End a frame — close command recording and submit. GL: no-op.
     */
    virtual void EndFrame() = 0;

    /**
     * @brief Present the finished frame to the window. GL: no-op until
     * present ownership is unified (the app still swaps GL buffers).
     */
    virtual void Present() = 0;

    /**
     * @brief Notify the backend of a drawable-size change (swapchain
     * recreation on Vulkan). GL: no-op (viewport handled per-pass).
     */
    virtual void Resize( int width, int height ) = 0;

    /**
     * @brief Save the current framebuffer to an image file (builtin
     * screenshot feature).
     *
     * Reads back the presented/default framebuffer and writes top-down
     * 8-bit RGB to @p path — PNG when the path ends in ".png", binary
     * PPM otherwise (see lsScreenshot::saveRGB).
     *
     * Call AFTER Render()/Present() of the frame you want captured.
     * Vulkan: reads back the last presented swapchain image.
     * OpenGL: glReadPixels on the default framebuffer (pre-swap).
     *
     * @param path output file path (".png" or ".ppm")
     * @return true on success
     */
    virtual bool Screenshot( const char* path ) = 0;

    /**
     * @brief Drain and log any backend-reported errors.
     *
     * @param name Label to identify where the check is being made.
     */
    virtual void CheckErrors( const string& name ) = 0;

    /**
     * @brief Save the current framebuffer binding so it can be restored.
     *
     * Called by passes that render to an FBO and then need to return to
     * the host framebuffer.
     */
    virtual void StoreDefaultFramebuffer() = 0;

    /**
     * @brief Restore the framebuffer binding saved by StoreDefaultFramebuffer().
     */
    virtual void RestoreDefaultFramebuffer() = 0;

    //=========================================================================
    // Texture objects
    //=========================================================================

    virtual void GenTextures( lsSizei n, lsTextureID* textures ) = 0;
    virtual void DeleteTextures( lsSizei n, lsTextureID* textures ) = 0;
    virtual void BindTexture( lsEnum target, lsTextureID texture ) = 0;
    virtual void ActiveTexture( lsEnum texture ) = 0;
    virtual void TexImage2D( lsEnum target, lsInt level, lsInt internalformat,
                             lsSizei width, lsSizei height, lsInt border,
                             lsEnum format, lsEnum type, const void* data ) = 0;
    virtual void TexParameteri( lsEnum target, lsEnum pname, lsInt param ) = 0;
    virtual void TexParameterf( lsEnum target, lsEnum pname, lsFloat param ) = 0;
    virtual void TexParameterfv( lsEnum target, lsEnum pname, const lsFloat* params ) = 0;
    virtual void GenerateMipmap( lsEnum target ) = 0;
    virtual void PixelStorei( lsEnum pname, lsInt param ) = 0;

    //=========================================================================
    // Framebuffer objects
    //=========================================================================

    virtual void GenFramebuffers( lsSizei n, lsFBOID* fbos ) = 0;
    virtual void DeleteFramebuffers( lsSizei n, lsFBOID* fbos ) = 0;
    virtual void BindFramebuffer( lsEnum target, lsFBOID fbo ) = 0;
    virtual void FramebufferTexture2D( lsEnum target, lsEnum attachment,
                                       lsEnum textarget, lsTextureID texture,
                                       lsInt level ) = 0;
    virtual lsEnum CheckFramebufferStatus( lsEnum target ) = 0;
    virtual void DrawBuffer( lsEnum mode ) = 0;
    virtual void ReadBuffer( lsEnum mode ) = 0;
    virtual void ReadPixels( lsInt x, lsInt y, lsSizei width, lsSizei height,
                             lsEnum format, lsEnum type, void* data ) = 0;

    //=========================================================================
    // Buffer / VAO objects
    //=========================================================================

    virtual void GenBuffers( lsSizei n, lsBufferID* buffers ) = 0;
    virtual void DeleteBuffers( lsSizei n, lsBufferID* buffers ) = 0;
    virtual void BindBuffer( lsEnum target, lsBufferID buffer ) = 0;
    virtual void BufferData( lsEnum target, lsSizeiptr size,
                             const void* data, lsEnum usage ) = 0;
    virtual void GenVertexArrays( lsSizei n, lsVAOID* arrays ) = 0;
    virtual void DeleteVertexArrays( lsSizei n, lsVAOID* arrays ) = 0;
    virtual void BindVertexArray( lsVAOID vao ) = 0;
    virtual void EnableVertexAttribArray( lsUInt index ) = 0;
    virtual void VertexAttribPointer( lsUInt index, lsInt size, lsEnum type,
                                      lsBoolean normalized, lsSizei stride,
                                      const void* pointer ) = 0;

    //=========================================================================
    // Draw calls
    //=========================================================================

    virtual void DrawArrays( lsEnum mode, lsInt first, lsSizei count ) = 0;
    virtual void DrawElements( lsEnum mode, lsSizei count, lsEnum type,
                               const void* indices ) = 0;

    //=========================================================================
    // Shader / program objects
    //=========================================================================

    virtual lsShaderID CreateShader( lsEnum type ) = 0;
    virtual lsProgramID CreateProgram() = 0;
    virtual void ShaderSource( lsShaderID shader, lsSizei count,
                               const lsChar** string, const lsInt* length ) = 0;
    virtual void CompileShader( lsShaderID shader ) = 0;
    virtual void AttachShader( lsProgramID program, lsShaderID shader ) = 0;
    virtual void DetachShader( lsProgramID program, lsShaderID shader ) = 0;
    virtual void LinkProgram( lsProgramID program ) = 0;
    virtual void UseProgram( lsProgramID program ) = 0;
    virtual void DeleteShader( lsShaderID shader ) = 0;
    virtual void DeleteProgram( lsProgramID program ) = 0;
    virtual lsUniformID GetUniformLocation( lsProgramID program, const lsChar* name ) = 0;
    virtual void GetShaderiv( lsShaderID shader, lsEnum pname, lsInt* params ) = 0;
    virtual void GetShaderInfoLog( lsShaderID shader, lsSizei bufSize,
                                   lsSizei* length, lsChar* infoLog ) = 0;
    virtual void GetProgramiv( lsProgramID program, lsEnum pname, lsInt* params ) = 0;
    virtual void GetProgramInfoLog( lsProgramID program, lsSizei bufSize,
                                    lsSizei* length, lsChar* infoLog ) = 0;

    //=========================================================================
    // Uniform setters
    //=========================================================================

    virtual void Uniform1i( lsUniformID location, lsInt v0 ) = 0;
    virtual void Uniform1f( lsUniformID location, lsFloat v0 ) = 0;
    virtual void Uniform3f( lsUniformID location, lsFloat v0, lsFloat v1, lsFloat v2 ) = 0;
    virtual void Uniform4f( lsUniformID location, lsFloat v0, lsFloat v1, lsFloat v2, lsFloat v3 ) = 0;
    virtual void UniformMatrix3fv( lsUniformID location, lsSizei count,
                                   lsBoolean transpose, const lsFloat* value ) = 0;
    virtual void UniformMatrix4fv( lsUniformID location, lsSizei count,
                                   lsBoolean transpose, const lsFloat* value ) = 0;

    //=========================================================================
    // Pipeline / rasterizer state
    //=========================================================================

    virtual void Enable( lsEnum cap ) = 0;
    virtual void Disable( lsEnum cap ) = 0;
    virtual void CullFace( lsEnum mode ) = 0;
    virtual void BlendFunc( lsEnum sfactor, lsEnum dfactor ) = 0;
    virtual void DepthFunc( lsEnum func ) = 0;
    virtual void DepthMask( lsBoolean flag ) = 0;
    virtual void DepthRangef( lsFloat near, lsFloat far ) = 0;
    virtual void ColorMask( lsBoolean r, lsBoolean g, lsBoolean b, lsBoolean a ) = 0;
    virtual void PolygonOffset( lsFloat factor, lsFloat units ) = 0;
    virtual void ShadeModel( lsEnum mode ) = 0;
    virtual void Viewport( lsInt x, lsInt y, lsSizei w, lsSizei h ) = 0;

    //=========================================================================
    // Buffer clear
    //=========================================================================

    virtual void Clear( lsBitfield mask ) = 0;
    virtual void ClearColor( lsFloat r, lsFloat g, lsFloat b, lsFloat a ) = 0;
    virtual void ClearDepth( lsDouble d ) = 0;

    //=========================================================================
    // Occlusion / timer queries
    //=========================================================================

    virtual void GenQueries( lsSizei n, lsQueryID* ids ) = 0;
    virtual void DeleteQueries( lsSizei n, lsQueryID* ids ) = 0;
    virtual void BeginQuery( lsEnum target, lsQueryID id ) = 0;
    virtual void EndQuery( lsEnum target ) = 0;
    virtual void GetQueryObjectiv( lsQueryID id, lsEnum pname, lsInt* params ) = 0;

    //=========================================================================
    // Diagnostic getters
    //=========================================================================

    virtual lsEnum GetError() = 0;
    virtual void GetIntegerv( lsEnum pname, lsInt* params ) = 0;

    //=========================================================================
    // Renderer-type and manager accessors (not backend-specific)
    //=========================================================================

    /**
     * @brief Get performance statistics.
     * @return Pointer to the lsStats instance, or nullptr.
     */
    lsStats* getStats() const { return m_stats; }

    /**
     * @brief Attach the performance statistics tracker.
     * @param val Pointer to lsStats (not owned by the renderer).
     */
    void setStats( lsStats* val ) { m_stats = val; }

    /**
     * @brief Get the shader manager.
     * @return Pointer to the lsShaderManager instance, or nullptr.
     */
    lsShaderManager* getShaderManager() const { return m_shaderManager; }

    /**
     * @brief Attach the shader manager.
     * @param val Pointer to lsShaderManager (not owned by the renderer).
     */
    void setShaderManager( lsShaderManager* val ) { m_shaderManager = val; }

    /**
     * @brief Set the concrete renderer type.
     * @param val RenderType value.
     */
    void setType( RenderType val ) { m_type = val; }

    /**
     * @brief Get the concrete renderer type.
     */
    RenderType getType() const { return m_type; }

protected:
    /** @brief Concrete renderer type — set by the subclass constructor. */
    RenderType m_type;

    /** @brief Performance statistics tracker (not owned). */
    lsStats* m_stats;

    /** @brief Shader manager (not owned). */
    lsShaderManager* m_shaderManager;
};
