/**
 * @file lsVulkan.h
 * @brief Vulkan backend for the lsRender interface — PLACEHOLDER SKELETON
 *
 * ⚠️  THIS CLASS IS A REMINDER / SCAFFOLD ONLY. Nothing is implemented.
 *
 * It exists so the intended second backend has a concrete home and the
 * shape of the work is visible in the tree: every pure-virtual method
 * of `lsRender` is overridden here with an empty stub body in
 * `lsVulkan.cpp`. See `docs/plans/renderer_abstraction.md` §10 ("Adding a Vulkan or Metal
 * backend") for the full recipe:
 *
 *   1. Implement every method below, translating engine-level types
 *      (`lsEnum`, `lsTextureID`, …) and `LS_*` constants into Vulkan
 *      API calls (VkInstance, VkDevice, VkCommandBuffer, …).
 *   2. Rewrite `render/lsRenderConstants.h` to map `LS_X` to the
 *      corresponding `Vk*` constants for this backend.
 *   3. No other engine source file needs to change.
 *
 * Instantiating this class is safe but useless: the constructor prints
 * a warning and every method is a no-op returning a default value.
 *
 * @see lsRender for the abstract interface contract.
 * @see lsOpenGL for the reference implementation.
 */

#pragma once

#include <iostream>

#include "lsRender.h"

class lsVulkan : public lsRender
{
public:
    lsVulkan();
    virtual ~lsVulkan();

    void Init(  ) override;  ///< TODO(vulkan): implement
    void CheckErrors( const string& name ) override;  ///< TODO(vulkan): implement
    void StoreDefaultFramebuffer(  ) override;  ///< TODO(vulkan): implement
    void RestoreDefaultFramebuffer(  ) override;  ///< TODO(vulkan): implement
    void GenTextures( lsSizei n, lsTextureID* textures ) override;  ///< TODO(vulkan): implement
    void DeleteTextures( lsSizei n, lsTextureID* textures ) override;  ///< TODO(vulkan): implement
    void BindTexture( lsEnum target, lsTextureID texture ) override;  ///< TODO(vulkan): implement
    void ActiveTexture( lsEnum texture ) override;  ///< TODO(vulkan): implement
    void TexParameteri( lsEnum target, lsEnum pname, lsInt param ) override;  ///< TODO(vulkan): implement
    void TexParameterf( lsEnum target, lsEnum pname, lsFloat param ) override;  ///< TODO(vulkan): implement
    void TexParameterfv( lsEnum target, lsEnum pname, const lsFloat* params ) override;  ///< TODO(vulkan): implement
    void GenerateMipmap( lsEnum target ) override;  ///< TODO(vulkan): implement
    void PixelStorei( lsEnum pname, lsInt param ) override;  ///< TODO(vulkan): implement
    void GenFramebuffers( lsSizei n, lsFBOID* fbos ) override;  ///< TODO(vulkan): implement
    void DeleteFramebuffers( lsSizei n, lsFBOID* fbos ) override;  ///< TODO(vulkan): implement
    void BindFramebuffer( lsEnum target, lsFBOID fbo ) override;  ///< TODO(vulkan): implement
    lsEnum CheckFramebufferStatus( lsEnum target ) override;  ///< TODO(vulkan): implement
    void DrawBuffer( lsEnum mode ) override;  ///< TODO(vulkan): implement
    void ReadBuffer( lsEnum mode ) override;  ///< TODO(vulkan): implement
    void GenBuffers( lsSizei n, lsBufferID* buffers ) override;  ///< TODO(vulkan): implement
    void DeleteBuffers( lsSizei n, lsBufferID* buffers ) override;  ///< TODO(vulkan): implement
    void BindBuffer( lsEnum target, lsBufferID buffer ) override;  ///< TODO(vulkan): implement
    void GenVertexArrays( lsSizei n, lsVAOID* arrays ) override;  ///< TODO(vulkan): implement
    void DeleteVertexArrays( lsSizei n, lsVAOID* arrays ) override;  ///< TODO(vulkan): implement
    void BindVertexArray( lsVAOID vao ) override;  ///< TODO(vulkan): implement
    void EnableVertexAttribArray( lsUInt index ) override;  ///< TODO(vulkan): implement
    void DrawArrays( lsEnum mode, lsInt first, lsSizei count ) override;  ///< TODO(vulkan): implement
    lsShaderID CreateShader( lsEnum type ) override;  ///< TODO(vulkan): implement
    lsProgramID CreateProgram(  ) override;  ///< TODO(vulkan): implement
    void CompileShader( lsShaderID shader ) override;  ///< TODO(vulkan): implement
    void AttachShader( lsProgramID program, lsShaderID shader ) override;  ///< TODO(vulkan): implement
    void DetachShader( lsProgramID program, lsShaderID shader ) override;  ///< TODO(vulkan): implement
    void LinkProgram( lsProgramID program ) override;  ///< TODO(vulkan): implement
    void UseProgram( lsProgramID program ) override;  ///< TODO(vulkan): implement
    void DeleteShader( lsShaderID shader ) override;  ///< TODO(vulkan): implement
    void DeleteProgram( lsProgramID program ) override;  ///< TODO(vulkan): implement
    lsUniformID GetUniformLocation( lsProgramID program, const lsChar* name ) override;  ///< TODO(vulkan): implement
    void GetShaderiv( lsShaderID shader, lsEnum pname, lsInt* params ) override;  ///< TODO(vulkan): implement
    void GetProgramiv( lsProgramID program, lsEnum pname, lsInt* params ) override;  ///< TODO(vulkan): implement
    void Uniform1i( lsUniformID location, lsInt v0 ) override;  ///< TODO(vulkan): implement
    void Uniform1f( lsUniformID location, lsFloat v0 ) override;  ///< TODO(vulkan): implement
    void Uniform3f( lsUniformID location, lsFloat v0, lsFloat v1, lsFloat v2 ) override;  ///< TODO(vulkan): implement
    void Uniform4f( lsUniformID location, lsFloat v0, lsFloat v1, lsFloat v2, lsFloat v3 ) override;  ///< TODO(vulkan): implement
    void Enable( lsEnum cap ) override;  ///< TODO(vulkan): implement
    void Disable( lsEnum cap ) override;  ///< TODO(vulkan): implement
    void CullFace( lsEnum mode ) override;  ///< TODO(vulkan): implement
    void BlendFunc( lsEnum sfactor, lsEnum dfactor ) override;  ///< TODO(vulkan): implement
    void DepthFunc( lsEnum func ) override;  ///< TODO(vulkan): implement
    void DepthMask( lsBoolean flag ) override;  ///< TODO(vulkan): implement
    void DepthRangef( lsFloat near, lsFloat far ) override;  ///< TODO(vulkan): implement
    void ColorMask( lsBoolean r, lsBoolean g, lsBoolean b, lsBoolean a ) override;  ///< TODO(vulkan): implement
    void PolygonOffset( lsFloat factor, lsFloat units ) override;  ///< TODO(vulkan): implement
    void ShadeModel( lsEnum mode ) override;  ///< TODO(vulkan): implement
    void Viewport( lsInt x, lsInt y, lsSizei w, lsSizei h ) override;  ///< TODO(vulkan): implement
    void Clear( lsBitfield mask ) override;  ///< TODO(vulkan): implement
    void ClearColor( lsFloat r, lsFloat g, lsFloat b, lsFloat a ) override;  ///< TODO(vulkan): implement
    void ClearDepth( lsDouble d ) override;  ///< TODO(vulkan): implement
    void GenQueries( lsSizei n, lsQueryID* ids ) override;  ///< TODO(vulkan): implement
    void DeleteQueries( lsSizei n, lsQueryID* ids ) override;  ///< TODO(vulkan): implement
    void BeginQuery( lsEnum target, lsQueryID id ) override;  ///< TODO(vulkan): implement
    void EndQuery( lsEnum target ) override;  ///< TODO(vulkan): implement
    void GetQueryObjectiv( lsQueryID id, lsEnum pname, lsInt* params ) override;  ///< TODO(vulkan): implement
    lsEnum GetError(  ) override;  ///< TODO(vulkan): implement
    void GetIntegerv( lsEnum pname, lsInt* params ) override;  ///< TODO(vulkan): implement
};
