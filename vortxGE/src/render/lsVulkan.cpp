/**
 * @file lsVulkan.cpp
 * @brief Stub bodies for the lsVulkan placeholder backend.
 *
 * ⚠️  NOTHING HERE IS IMPLEMENTED. Every method is a no-op (non-void
 * methods return a default-constructed value). This file exists purely
 * as a scaffold/reminder for the future Vulkan backend — see
 * `lsVulkan.h` and `docs/plans/renderer_abstraction.md` §10.
 */

#include "lsVulkan.h"

lsVulkan::lsVulkan()
{
    setType( RenderType::Vulkan );
    std::cout << "WARNING: lsVulkan is a placeholder skeleton — "
                 "no methods are implemented." << std::endl;
}

lsVulkan::~lsVulkan() = default;

void lsVulkan::Init(  )
{
    // TODO(vulkan): implement
}

void lsVulkan::CheckErrors( const string& name )
{
    (void)name;
    // TODO(vulkan): implement
}

void lsVulkan::StoreDefaultFramebuffer(  )
{
    // TODO(vulkan): implement
}

void lsVulkan::RestoreDefaultFramebuffer(  )
{
    // TODO(vulkan): implement
}

void lsVulkan::GenTextures( lsSizei n, lsTextureID* textures )
{
    (void)n; (void)textures;
    // TODO(vulkan): implement
}

void lsVulkan::DeleteTextures( lsSizei n, lsTextureID* textures )
{
    (void)n; (void)textures;
    // TODO(vulkan): implement
}

void lsVulkan::BindTexture( lsEnum target, lsTextureID texture )
{
    (void)target; (void)texture;
    // TODO(vulkan): implement
}

void lsVulkan::ActiveTexture( lsEnum texture )
{
    (void)texture;
    // TODO(vulkan): implement
}

void lsVulkan::TexParameteri( lsEnum target, lsEnum pname, lsInt param )
{
    (void)target; (void)pname; (void)param;
    // TODO(vulkan): implement
}

void lsVulkan::TexParameterf( lsEnum target, lsEnum pname, lsFloat param )
{
    (void)target; (void)pname; (void)param;
    // TODO(vulkan): implement
}

void lsVulkan::TexParameterfv( lsEnum target, lsEnum pname, const lsFloat* params )
{
    (void)target; (void)pname; (void)params;
    // TODO(vulkan): implement
}

void lsVulkan::GenerateMipmap( lsEnum target )
{
    (void)target;
    // TODO(vulkan): implement
}

void lsVulkan::PixelStorei( lsEnum pname, lsInt param )
{
    (void)pname; (void)param;
    // TODO(vulkan): implement
}

void lsVulkan::GenFramebuffers( lsSizei n, lsFBOID* fbos )
{
    (void)n; (void)fbos;
    // TODO(vulkan): implement
}

void lsVulkan::DeleteFramebuffers( lsSizei n, lsFBOID* fbos )
{
    (void)n; (void)fbos;
    // TODO(vulkan): implement
}

void lsVulkan::BindFramebuffer( lsEnum target, lsFBOID fbo )
{
    (void)target; (void)fbo;
    // TODO(vulkan): implement
}

lsEnum lsVulkan::CheckFramebufferStatus( lsEnum target )
{
    (void)target;
    // TODO(vulkan): implement
    return lsEnum{};
}

void lsVulkan::DrawBuffer( lsEnum mode )
{
    (void)mode;
    // TODO(vulkan): implement
}

void lsVulkan::ReadBuffer( lsEnum mode )
{
    (void)mode;
    // TODO(vulkan): implement
}

void lsVulkan::GenBuffers( lsSizei n, lsBufferID* buffers )
{
    (void)n; (void)buffers;
    // TODO(vulkan): implement
}

void lsVulkan::DeleteBuffers( lsSizei n, lsBufferID* buffers )
{
    (void)n; (void)buffers;
    // TODO(vulkan): implement
}

void lsVulkan::BindBuffer( lsEnum target, lsBufferID buffer )
{
    (void)target; (void)buffer;
    // TODO(vulkan): implement
}

void lsVulkan::GenVertexArrays( lsSizei n, lsVAOID* arrays )
{
    (void)n; (void)arrays;
    // TODO(vulkan): implement
}

void lsVulkan::DeleteVertexArrays( lsSizei n, lsVAOID* arrays )
{
    (void)n; (void)arrays;
    // TODO(vulkan): implement
}

void lsVulkan::BindVertexArray( lsVAOID vao )
{
    (void)vao;
    // TODO(vulkan): implement
}

void lsVulkan::EnableVertexAttribArray( lsUInt index )
{
    (void)index;
    // TODO(vulkan): implement
}

void lsVulkan::DrawArrays( lsEnum mode, lsInt first, lsSizei count )
{
    (void)mode; (void)first; (void)count;
    // TODO(vulkan): implement
}

lsShaderID lsVulkan::CreateShader( lsEnum type )
{
    (void)type;
    // TODO(vulkan): implement
    return lsShaderID{};
}

lsProgramID lsVulkan::CreateProgram(  )
{
    // TODO(vulkan): implement
    return lsProgramID{};
}

void lsVulkan::CompileShader( lsShaderID shader )
{
    (void)shader;
    // TODO(vulkan): implement
}

void lsVulkan::AttachShader( lsProgramID program, lsShaderID shader )
{
    (void)program; (void)shader;
    // TODO(vulkan): implement
}

void lsVulkan::DetachShader( lsProgramID program, lsShaderID shader )
{
    (void)program; (void)shader;
    // TODO(vulkan): implement
}

void lsVulkan::LinkProgram( lsProgramID program )
{
    (void)program;
    // TODO(vulkan): implement
}

void lsVulkan::UseProgram( lsProgramID program )
{
    (void)program;
    // TODO(vulkan): implement
}

void lsVulkan::DeleteShader( lsShaderID shader )
{
    (void)shader;
    // TODO(vulkan): implement
}

void lsVulkan::DeleteProgram( lsProgramID program )
{
    (void)program;
    // TODO(vulkan): implement
}

lsUniformID lsVulkan::GetUniformLocation( lsProgramID program, const lsChar* name )
{
    (void)program; (void)name;
    // TODO(vulkan): implement
    return lsUniformID{};
}

void lsVulkan::GetShaderiv( lsShaderID shader, lsEnum pname, lsInt* params )
{
    (void)shader; (void)pname; (void)params;
    // TODO(vulkan): implement
}

void lsVulkan::GetProgramiv( lsProgramID program, lsEnum pname, lsInt* params )
{
    (void)program; (void)pname; (void)params;
    // TODO(vulkan): implement
}

void lsVulkan::Uniform1i( lsUniformID location, lsInt v0 )
{
    (void)location; (void)v0;
    // TODO(vulkan): implement
}

void lsVulkan::Uniform1f( lsUniformID location, lsFloat v0 )
{
    (void)location; (void)v0;
    // TODO(vulkan): implement
}

void lsVulkan::Uniform3f( lsUniformID location, lsFloat v0, lsFloat v1, lsFloat v2 )
{
    (void)location; (void)v0; (void)v1; (void)v2;
    // TODO(vulkan): implement
}

void lsVulkan::Uniform4f( lsUniformID location, lsFloat v0, lsFloat v1, lsFloat v2, lsFloat v3 )
{
    (void)location; (void)v0; (void)v1; (void)v2; (void)v3;
    // TODO(vulkan): implement
}

void lsVulkan::Enable( lsEnum cap )
{
    (void)cap;
    // TODO(vulkan): implement
}

void lsVulkan::Disable( lsEnum cap )
{
    (void)cap;
    // TODO(vulkan): implement
}

void lsVulkan::CullFace( lsEnum mode )
{
    (void)mode;
    // TODO(vulkan): implement
}

void lsVulkan::BlendFunc( lsEnum sfactor, lsEnum dfactor )
{
    (void)sfactor; (void)dfactor;
    // TODO(vulkan): implement
}

void lsVulkan::DepthFunc( lsEnum func )
{
    (void)func;
    // TODO(vulkan): implement
}

void lsVulkan::DepthMask( lsBoolean flag )
{
    (void)flag;
    // TODO(vulkan): implement
}

void lsVulkan::DepthRangef( lsFloat near, lsFloat far )
{
    (void)near; (void)far;
    // TODO(vulkan): implement
}

void lsVulkan::ColorMask( lsBoolean r, lsBoolean g, lsBoolean b, lsBoolean a )
{
    (void)r; (void)g; (void)b; (void)a;
    // TODO(vulkan): implement
}

void lsVulkan::PolygonOffset( lsFloat factor, lsFloat units )
{
    (void)factor; (void)units;
    // TODO(vulkan): implement
}

void lsVulkan::ShadeModel( lsEnum mode )
{
    (void)mode;
    // TODO(vulkan): implement
}

void lsVulkan::Viewport( lsInt x, lsInt y, lsSizei w, lsSizei h )
{
    (void)x; (void)y; (void)w; (void)h;
    // TODO(vulkan): implement
}

void lsVulkan::Clear( lsBitfield mask )
{
    (void)mask;
    // TODO(vulkan): implement
}

void lsVulkan::ClearColor( lsFloat r, lsFloat g, lsFloat b, lsFloat a )
{
    (void)r; (void)g; (void)b; (void)a;
    // TODO(vulkan): implement
}

void lsVulkan::ClearDepth( lsDouble d )
{
    (void)d;
    // TODO(vulkan): implement
}

void lsVulkan::GenQueries( lsSizei n, lsQueryID* ids )
{
    (void)n; (void)ids;
    // TODO(vulkan): implement
}

void lsVulkan::DeleteQueries( lsSizei n, lsQueryID* ids )
{
    (void)n; (void)ids;
    // TODO(vulkan): implement
}

void lsVulkan::BeginQuery( lsEnum target, lsQueryID id )
{
    (void)target; (void)id;
    // TODO(vulkan): implement
}

void lsVulkan::EndQuery( lsEnum target )
{
    (void)target;
    // TODO(vulkan): implement
}

void lsVulkan::GetQueryObjectiv( lsQueryID id, lsEnum pname, lsInt* params )
{
    (void)id; (void)pname; (void)params;
    // TODO(vulkan): implement
}

lsEnum lsVulkan::GetError(  )
{
    // TODO(vulkan): implement
    return lsEnum{};
}

void lsVulkan::GetIntegerv( lsEnum pname, lsInt* params )
{
    (void)pname; (void)params;
    // TODO(vulkan): implement
}

