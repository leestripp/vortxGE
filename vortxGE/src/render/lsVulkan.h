/**
 * @file lsVulkan.h
 * @brief Vulkan implementation of the lsRender interface (PRIMARY backend)
 *
 * Status: V0 skeleton (Plan.md Part 2). Every method is a stub;
 * Init() returns false so vortxGE falls back to lsOpenGL until the
 * V1 milestone (instance/device/surface/swapchain/clear) lands.
 *
 * Design rules (see Plan.md Part 2):
 * - LS_* constants keep their GL numeric values; this backend maps
 *   them to Vk* enums at its boundary (lsVulkanTypes.h, added in V1).
 * - The engine never includes <vulkan/vulkan.h> outside this pair of
 *   files (same rule as <GL/glew.h> for lsOpenGL).
 */
#pragma once

#include "lsRender.h"

/**
 * @class lsVulkan
 * @brief Concrete Vulkan backend (primary renderer).
 */
class lsVulkan : public lsRender
{
public:
	lsVulkan();
	virtual ~lsVulkan();
	bool Init(  ) override;
	void SetWindow( void* nativeWindow ) override;
	void BeginFrame(  ) override;
	void EndFrame(  ) override;
	void Present(  ) override;
	void Resize( int width, int height ) override;
	void CheckErrors( const string& name ) override;
	void StoreDefaultFramebuffer(  ) override;
	void RestoreDefaultFramebuffer(  ) override;
	void GenTextures( lsSizei n, lsTextureID* textures ) override;
	void DeleteTextures( lsSizei n, lsTextureID* textures ) override;
	void BindTexture( lsEnum target, lsTextureID texture ) override;
	void ActiveTexture( lsEnum texture ) override;
	void TexImage2D( lsEnum target, lsInt level, lsInt internalformat, lsSizei width, lsSizei height, lsInt border, lsEnum format, lsEnum type, const void* data ) override;
	void TexParameteri( lsEnum target, lsEnum pname, lsInt param ) override;
	void TexParameterf( lsEnum target, lsEnum pname, lsFloat param ) override;
	void TexParameterfv( lsEnum target, lsEnum pname, const lsFloat* params ) override;
	void GenerateMipmap( lsEnum target ) override;
	void PixelStorei( lsEnum pname, lsInt param ) override;
	void GenFramebuffers( lsSizei n, lsFBOID* fbos ) override;
	void DeleteFramebuffers( lsSizei n, lsFBOID* fbos ) override;
	void BindFramebuffer( lsEnum target, lsFBOID fbo ) override;
	void FramebufferTexture2D( lsEnum target, lsEnum attachment, lsEnum textarget, lsTextureID texture, lsInt level ) override;
	lsEnum CheckFramebufferStatus( lsEnum target ) override;
	void DrawBuffer( lsEnum mode ) override;
	void ReadBuffer( lsEnum mode ) override;
	void ReadPixels( lsInt x, lsInt y, lsSizei width, lsSizei height, lsEnum format, lsEnum type, void* data ) override;
	void GenBuffers( lsSizei n, lsBufferID* buffers ) override;
	void DeleteBuffers( lsSizei n, lsBufferID* buffers ) override;
	void BindBuffer( lsEnum target, lsBufferID buffer ) override;
	void BufferData( lsEnum target, lsSizeiptr size, const void* data, lsEnum usage ) override;
	void GenVertexArrays( lsSizei n, lsVAOID* arrays ) override;
	void DeleteVertexArrays( lsSizei n, lsVAOID* arrays ) override;
	void BindVertexArray( lsVAOID vao ) override;
	void EnableVertexAttribArray( lsUInt index ) override;
	void VertexAttribPointer( lsUInt index, lsInt size, lsEnum type, lsBoolean normalized, lsSizei stride, const void* pointer ) override;
	void DrawArrays( lsEnum mode, lsInt first, lsSizei count ) override;
	void DrawElements( lsEnum mode, lsSizei count, lsEnum type, const void* indices ) override;
	lsShaderID CreateShader( lsEnum type ) override;
	lsProgramID CreateProgram(  ) override;
	void ShaderSource( lsShaderID shader, lsSizei count, const lsChar** string, const lsInt* length ) override;
	void CompileShader( lsShaderID shader ) override;
	void AttachShader( lsProgramID program, lsShaderID shader ) override;
	void DetachShader( lsProgramID program, lsShaderID shader ) override;
	void LinkProgram( lsProgramID program ) override;
	void UseProgram( lsProgramID program ) override;
	void DeleteShader( lsShaderID shader ) override;
	void DeleteProgram( lsProgramID program ) override;
	lsUniformID GetUniformLocation( lsProgramID program, const lsChar* name ) override;
	void GetShaderiv( lsShaderID shader, lsEnum pname, lsInt* params ) override;
	void GetShaderInfoLog( lsShaderID shader, lsSizei bufSize, lsSizei* length, lsChar* infoLog ) override;
	void GetProgramiv( lsProgramID program, lsEnum pname, lsInt* params ) override;
	void GetProgramInfoLog( lsProgramID program, lsSizei bufSize, lsSizei* length, lsChar* infoLog ) override;
	void Uniform1i( lsUniformID location, lsInt v0 ) override;
	void Uniform1f( lsUniformID location, lsFloat v0 ) override;
	void Uniform3f( lsUniformID location, lsFloat v0, lsFloat v1, lsFloat v2 ) override;
	void Uniform4f( lsUniformID location, lsFloat v0, lsFloat v1, lsFloat v2, lsFloat v3 ) override;
	void UniformMatrix3fv( lsUniformID location, lsSizei count, lsBoolean transpose, const lsFloat* value ) override;
	void UniformMatrix4fv( lsUniformID location, lsSizei count, lsBoolean transpose, const lsFloat* value ) override;
	void Enable( lsEnum cap ) override;
	void Disable( lsEnum cap ) override;
	void CullFace( lsEnum mode ) override;
	void BlendFunc( lsEnum sfactor, lsEnum dfactor ) override;
	void DepthFunc( lsEnum func ) override;
	void DepthMask( lsBoolean flag ) override;
	void DepthRangef( lsFloat near, lsFloat far ) override;
	void ColorMask( lsBoolean r, lsBoolean g, lsBoolean b, lsBoolean a ) override;
	void PolygonOffset( lsFloat factor, lsFloat units ) override;
	void ShadeModel( lsEnum mode ) override;
	void Viewport( lsInt x, lsInt y, lsSizei w, lsSizei h ) override;
	void Clear( lsBitfield mask ) override;
	void ClearColor( lsFloat r, lsFloat g, lsFloat b, lsFloat a ) override;
	void ClearDepth( lsDouble d ) override;
	void GenQueries( lsSizei n, lsQueryID* ids ) override;
	void DeleteQueries( lsSizei n, lsQueryID* ids ) override;
	void BeginQuery( lsEnum target, lsQueryID id ) override;
	void EndQuery( lsEnum target ) override;
	void GetQueryObjectiv( lsQueryID id, lsEnum pname, lsInt* params ) override;
	lsEnum GetError(  ) override;
	void GetIntegerv( lsEnum pname, lsInt* params ) override;

private:
	// V1: VkInstance, VkDevice, VkSurfaceKHR, VkSwapchainKHR, ...
};
