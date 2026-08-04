/**
 * @file lsVulkan.cpp
 * @brief lsVulkan — V0 skeleton. All methods are stubs.
 *
 * GENERATED skeleton (from lsRender.h pure virtuals). As V1+ milestones
 * implement real bodies, keep the method ordering aligned with lsRender.h.
 */

#include "lsVulkan.h"

lsVulkan::lsVulkan()
{
	setType( RenderType::Vulkan );
}

lsVulkan::~lsVulkan()
{
	// V1: destroy swapchain, device, surface, instance (reverse order)
}
bool lsVulkan::Init(  )
{
	cout << "INFO: lsVulkan::Init() - not implemented yet (V0 skeleton)" << endl;
	return false;	// triggers vortxGE fallback to lsOpenGL
}

void lsVulkan::SetWindow( void* )
{
	// V1+ stub
}

void lsVulkan::BeginFrame(  )
{
	// V1+ stub
}

void lsVulkan::EndFrame(  )
{
	// V1+ stub
}

void lsVulkan::Present(  )
{
	// V1+ stub
}

void lsVulkan::Resize( int, int )
{
	// V1+ stub
}

void lsVulkan::CheckErrors( const string& )
{
	// V1+ stub
}

void lsVulkan::StoreDefaultFramebuffer(  )
{
	// V1+ stub
}

void lsVulkan::RestoreDefaultFramebuffer(  )
{
	// V1+ stub
}

void lsVulkan::GenTextures( lsSizei, lsTextureID* )
{
	// V1+ stub
}

void lsVulkan::DeleteTextures( lsSizei, lsTextureID* )
{
	// V1+ stub
}

void lsVulkan::BindTexture( lsEnum, lsTextureID )
{
	// V1+ stub
}

void lsVulkan::ActiveTexture( lsEnum )
{
	// V1+ stub
}

void lsVulkan::TexImage2D( lsEnum, lsInt, lsInt, lsSizei, lsSizei, lsInt, lsEnum, lsEnum, const void* )
{
	// V1+ stub
}

void lsVulkan::TexParameteri( lsEnum, lsEnum, lsInt )
{
	// V1+ stub
}

void lsVulkan::TexParameterf( lsEnum, lsEnum, lsFloat )
{
	// V1+ stub
}

void lsVulkan::TexParameterfv( lsEnum, lsEnum, const lsFloat* )
{
	// V1+ stub
}

void lsVulkan::GenerateMipmap( lsEnum )
{
	// V1+ stub
}

void lsVulkan::PixelStorei( lsEnum, lsInt )
{
	// V1+ stub
}

void lsVulkan::GenFramebuffers( lsSizei, lsFBOID* )
{
	// V1+ stub
}

void lsVulkan::DeleteFramebuffers( lsSizei, lsFBOID* )
{
	// V1+ stub
}

void lsVulkan::BindFramebuffer( lsEnum, lsFBOID )
{
	// V1+ stub
}

void lsVulkan::FramebufferTexture2D( lsEnum, lsEnum, lsEnum, lsTextureID, lsInt )
{
	// V1+ stub
}

lsEnum lsVulkan::CheckFramebufferStatus( lsEnum )
{
	return (lsEnum){};
}

void lsVulkan::DrawBuffer( lsEnum )
{
	// V1+ stub
}

void lsVulkan::ReadBuffer( lsEnum )
{
	// V1+ stub
}

void lsVulkan::ReadPixels( lsInt, lsInt, lsSizei, lsSizei, lsEnum, lsEnum, void* )
{
	// V1+ stub
}

void lsVulkan::GenBuffers( lsSizei, lsBufferID* )
{
	// V1+ stub
}

void lsVulkan::DeleteBuffers( lsSizei, lsBufferID* )
{
	// V1+ stub
}

void lsVulkan::BindBuffer( lsEnum, lsBufferID )
{
	// V1+ stub
}

void lsVulkan::BufferData( lsEnum, lsSizeiptr, const void*, lsEnum )
{
	// V1+ stub
}

void lsVulkan::GenVertexArrays( lsSizei, lsVAOID* )
{
	// V1+ stub
}

void lsVulkan::DeleteVertexArrays( lsSizei, lsVAOID* )
{
	// V1+ stub
}

void lsVulkan::BindVertexArray( lsVAOID )
{
	// V1+ stub
}

void lsVulkan::EnableVertexAttribArray( lsUInt )
{
	// V1+ stub
}

void lsVulkan::VertexAttribPointer( lsUInt, lsInt, lsEnum, lsBoolean, lsSizei, const void* )
{
	// V1+ stub
}

void lsVulkan::DrawArrays( lsEnum, lsInt, lsSizei )
{
	// V1+ stub
}

void lsVulkan::DrawElements( lsEnum, lsSizei, lsEnum, const void* )
{
	// V1+ stub
}

lsShaderID lsVulkan::CreateShader( lsEnum )
{
	return (lsShaderID){};
}

lsProgramID lsVulkan::CreateProgram(  )
{
	return (lsProgramID){};
}

void lsVulkan::ShaderSource( lsShaderID, lsSizei, const lsChar**, const lsInt* )
{
	// V1+ stub
}

void lsVulkan::CompileShader( lsShaderID )
{
	// V1+ stub
}

void lsVulkan::AttachShader( lsProgramID, lsShaderID )
{
	// V1+ stub
}

void lsVulkan::DetachShader( lsProgramID, lsShaderID )
{
	// V1+ stub
}

void lsVulkan::LinkProgram( lsProgramID )
{
	// V1+ stub
}

void lsVulkan::UseProgram( lsProgramID )
{
	// V1+ stub
}

void lsVulkan::DeleteShader( lsShaderID )
{
	// V1+ stub
}

void lsVulkan::DeleteProgram( lsProgramID )
{
	// V1+ stub
}

lsUniformID lsVulkan::GetUniformLocation( lsProgramID, const lsChar* )
{
	return (lsUniformID){};
}

void lsVulkan::GetShaderiv( lsShaderID, lsEnum, lsInt* )
{
	// V1+ stub
}

void lsVulkan::GetShaderInfoLog( lsShaderID, lsSizei, lsSizei*, lsChar* )
{
	// V1+ stub
}

void lsVulkan::GetProgramiv( lsProgramID, lsEnum, lsInt* )
{
	// V1+ stub
}

void lsVulkan::GetProgramInfoLog( lsProgramID, lsSizei, lsSizei*, lsChar* )
{
	// V1+ stub
}

void lsVulkan::Uniform1i( lsUniformID, lsInt )
{
	// V1+ stub
}

void lsVulkan::Uniform1f( lsUniformID, lsFloat )
{
	// V1+ stub
}

void lsVulkan::Uniform3f( lsUniformID, lsFloat, lsFloat, lsFloat )
{
	// V1+ stub
}

void lsVulkan::Uniform4f( lsUniformID, lsFloat, lsFloat, lsFloat, lsFloat )
{
	// V1+ stub
}

void lsVulkan::UniformMatrix3fv( lsUniformID, lsSizei, lsBoolean, const lsFloat* )
{
	// V1+ stub
}

void lsVulkan::UniformMatrix4fv( lsUniformID, lsSizei, lsBoolean, const lsFloat* )
{
	// V1+ stub
}

void lsVulkan::Enable( lsEnum )
{
	// V1+ stub
}

void lsVulkan::Disable( lsEnum )
{
	// V1+ stub
}

void lsVulkan::CullFace( lsEnum )
{
	// V1+ stub
}

void lsVulkan::BlendFunc( lsEnum, lsEnum )
{
	// V1+ stub
}

void lsVulkan::DepthFunc( lsEnum )
{
	// V1+ stub
}

void lsVulkan::DepthMask( lsBoolean )
{
	// V1+ stub
}

void lsVulkan::DepthRangef( lsFloat, lsFloat )
{
	// V1+ stub
}

void lsVulkan::ColorMask( lsBoolean, lsBoolean, lsBoolean, lsBoolean )
{
	// V1+ stub
}

void lsVulkan::PolygonOffset( lsFloat, lsFloat )
{
	// V1+ stub
}

void lsVulkan::ShadeModel( lsEnum )
{
	// V1+ stub
}

void lsVulkan::Viewport( lsInt, lsInt, lsSizei, lsSizei )
{
	// V1+ stub
}

void lsVulkan::Clear( lsBitfield )
{
	// V1+ stub
}

void lsVulkan::ClearColor( lsFloat, lsFloat, lsFloat, lsFloat )
{
	// V1+ stub
}

void lsVulkan::ClearDepth( lsDouble )
{
	// V1+ stub
}

void lsVulkan::GenQueries( lsSizei, lsQueryID* )
{
	// V1+ stub
}

void lsVulkan::DeleteQueries( lsSizei, lsQueryID* )
{
	// V1+ stub
}

void lsVulkan::BeginQuery( lsEnum, lsQueryID )
{
	// V1+ stub
}

void lsVulkan::EndQuery( lsEnum )
{
	// V1+ stub
}

void lsVulkan::GetQueryObjectiv( lsQueryID, lsEnum, lsInt* )
{
	// V1+ stub
}

lsEnum lsVulkan::GetError(  )
{
	return (lsEnum){};
}

void lsVulkan::GetIntegerv( lsEnum, lsInt* )
{
	// V1+ stub
}
