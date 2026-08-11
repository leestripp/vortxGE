/**
 * @file lsOpenGL.cpp
 * @brief lsOpenGL — bodies for every lsRender override
 *
 * Every method here is a one-line forward to the matching `gl*()`
 * function. The translation from engine-level types (`lsEnum`,
 * `lsTextureID`, etc.) to GL types (`GLenum`, `GLuint`, …) is implicit
 * because the underlying numeric types match.
 *
 * This is the **only** translation unit in the engine that calls
 * `gl*()` functions. See `lsRender.h` for the abstract interface and
 * `lsGLTypes.h` for the engine-level typedefs used in method
 * signatures.
 */

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "../vortx_debug.h"

// datablocks
#include "../datablocks/lsString.h"

#include "lsOpenGL.h"
#include "lsScreenshot.h"

#include <cstring>

//=========================================================================
// Construction / destruction
//=========================================================================

lsOpenGL::lsOpenGL() : lsRender()
{
	setType(RenderType::OpenGL);

	// Initialize OpenGL-specific members
	m_defaultFBO = 0;
}

lsOpenGL::~lsOpenGL()
{
}

//=========================================================================
// lsRender overrides — lifecycle
//=========================================================================

bool lsOpenGL::Init()
{
#ifdef VORTX_DEBUG_RENDER
	cout << "lsOpenGL::Init()" << endl;
#endif

	// Extensions
	// User needs to load extensions e.g. GLEW

	// collect info
	GetInfo();

	return true;	// context is app-created; if it were bad we'd have failed earlier
}

// ---------------------------------------------------------------------
// Frame lifecycle shims (no-ops — GL context/swap owned by the app)
// ---------------------------------------------------------------------
void lsOpenGL::SetWindow( void* nativeWindow ) { (void)nativeWindow; }
void lsOpenGL::BeginFrame() {}
void lsOpenGL::EndFrame()
{
	// VORTX_SHOT: capture the framebuffer at frame 30 (same convention
	// as the Vulkan backend) for GL<->Vulkan / legacy<->pipeline
	// side-by-side comparison. One-shot.
	static const char* shotPath = getenv( "VORTX_SHOT" );
	if( !shotPath ) return;
	static int s_frames = 30;
	if( s_frames <= 0 || --s_frames != 0 ) return;
	Screenshot( shotPath );
}

bool lsOpenGL::Screenshot( const char* path )
{
	GLint vp[4] = { 0, 0, 0, 0 };
	glGetIntegerv( GL_VIEWPORT, vp );
	const int w = vp[2], h = vp[3];
	if( w <= 0 || h <= 0 ) return false;

	std::vector<unsigned char> px( (size_t)w * (size_t)h * 3 );
	glPixelStorei( GL_PACK_ALIGNMENT, 1 );
	glReadPixels( 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, px.data() );

	// glReadPixels is bottom-up; lsScreenshot::saveRGB wants top-down
	std::vector<unsigned char> flipped( px.size() );
	const size_t stride = (size_t)w * 3;
	for( int y = 0; y < h; y++ )
		memcpy( flipped.data() + (size_t)y * stride,
				px.data() + (size_t)( h - 1 - y ) * stride, stride );
	return lsScreenshot::saveRGB( path, w, h, flipped.data() );
}
void lsOpenGL::Present() {}
void lsOpenGL::Resize( int width, int height ) { (void)width; (void)height; }

void lsOpenGL::CheckErrors( const string &name )
{
	GLenum err;
	while((err = glGetError()) != GL_NO_ERROR)
	{
		string errorMsg = "Unknown error";
		switch(err)
		{
			case GL_INVALID_ENUM:           errorMsg = "GL_INVALID_ENUM"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: errorMsg = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
			case GL_INVALID_INDEX:          errorMsg = "GL_INVALID_INDEX"; break;
			case GL_INVALID_OPERATION:      errorMsg = "GL_INVALID_OPERATION"; break;
			case GL_INVALID_VALUE:          errorMsg = "GL_INVALID_VALUE"; break;
		}
		cerr << "OpenGL ERROT : " << name << " : " << errorMsg << endl;
	}
}

void lsOpenGL::GetInfo()
{
	cout << "***** ******************************************" << endl;
	cout << "***** OpenGL Info" << endl;
	cout << "***** ******************************************" << endl;

	// Version
	cout << "* Version  : ";
	cout << glGetString(GL_VERSION) << endl;

	// Vender
	cout << "* Vender   : ";
	cout << glGetString( GL_VENDOR ) << endl;

	// Renderer
	cout << "* Renderer : ";
	cout << glGetString( GL_RENDERER ) << endl;

	// GL_SHADING_LANGUAGE_VERSION
	cout << "* Shaders  : ";
	cout << glGetString( GL_SHADING_LANGUAGE_VERSION ) << endl;

	cout << "***** ******************************************" << endl;
}

void lsOpenGL::StoreDefaultFramebuffer()
{
	// Find out default FBO
	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &m_defaultFBO );

#ifdef VORTX_DEBUG_RENDER
	cout << "lsOpenGL::StoreDefaultFramebuffer : " << m_defaultFBO << endl;
#endif
}

void lsOpenGL::RestoreDefaultFramebuffer()
{
#ifdef VORTX_DEBUG_RENDER
	cout << "lsOpenGL::RestoreDefaultFramebuffer : " << m_defaultFBO << endl;
#endif
	glBindFramebuffer( GL_FRAMEBUFFER, m_defaultFBO );
}

//=========================================================================
// lsRender overrides — texture objects
//=========================================================================
// Each body implicitly converts engine-level types (lsEnum, lsTextureID,
// lsFloat, etc.) to GL types (GLenum, GLuint, GLfloat, …) at the call
// boundary; the underlying numeric types match.

void lsOpenGL::GenTextures( lsSizei n, lsTextureID* textures )                              { glGenTextures( static_cast<GLsizei>(n), reinterpret_cast<GLuint*>(textures) ); }
void lsOpenGL::DeleteTextures( lsSizei n, lsTextureID* textures )                           { glDeleteTextures( static_cast<GLsizei>(n), reinterpret_cast<GLuint*>(textures) ); }
void lsOpenGL::BindTexture( lsEnum target, lsTextureID texture )                            { glBindTexture( static_cast<GLenum>(target), static_cast<GLuint>(texture) ); }
void lsOpenGL::ActiveTexture( lsEnum texture )                                              { glActiveTexture( static_cast<GLenum>(texture) ); }
void lsOpenGL::TexImage2D( lsEnum target, lsInt level, lsInt internalformat,
                           lsSizei width, lsSizei height, lsInt border,
                           lsEnum format, lsEnum type, const void* data )                  { glTexImage2D( static_cast<GLenum>(target), static_cast<GLint>(level), static_cast<GLint>(internalformat), static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLint>(border), static_cast<GLenum>(format), static_cast<GLenum>(type), data ); }
void lsOpenGL::TexParameteri( lsEnum target, lsEnum pname, lsInt param )                    { glTexParameteri( static_cast<GLenum>(target), static_cast<GLenum>(pname), static_cast<GLint>(param) ); }
void lsOpenGL::TexParameterf( lsEnum target, lsEnum pname, lsFloat param )                  { glTexParameterf( static_cast<GLenum>(target), static_cast<GLenum>(pname), static_cast<GLfloat>(param) ); }
void lsOpenGL::TexParameterfv( lsEnum target, lsEnum pname, const lsFloat* params )         { glTexParameterfv( static_cast<GLenum>(target), static_cast<GLenum>(pname), reinterpret_cast<const GLfloat*>(params) ); }
void lsOpenGL::GenerateMipmap( lsEnum target )                                             { glGenerateMipmap( static_cast<GLenum>(target) ); }
void lsOpenGL::PixelStorei( lsEnum pname, lsInt param )                                    { glPixelStorei( static_cast<GLenum>(pname), static_cast<GLint>(param) ); }

// ---- Framebuffer objects ----
void lsOpenGL::GenFramebuffers( lsSizei n, lsFBOID* fbos )                                  { glGenFramebuffers( static_cast<GLsizei>(n), reinterpret_cast<GLuint*>(fbos) ); }
void lsOpenGL::DeleteFramebuffers( lsSizei n, lsFBOID* fbos )                               { glDeleteFramebuffers( static_cast<GLsizei>(n), reinterpret_cast<GLuint*>(fbos) ); }
void lsOpenGL::BindFramebuffer( lsEnum target, lsFBOID fbo )                                { glBindFramebuffer( static_cast<GLenum>(target), static_cast<GLuint>(fbo) ); }
void lsOpenGL::FramebufferTexture2D( lsEnum target, lsEnum attachment,
                                     lsEnum textarget, lsTextureID texture, lsInt level ) { glFramebufferTexture2D( static_cast<GLenum>(target), static_cast<GLenum>(attachment), static_cast<GLenum>(textarget), static_cast<GLuint>(texture), static_cast<GLint>(level) ); }
lsEnum lsOpenGL::CheckFramebufferStatus( lsEnum target )                                   { return static_cast<lsEnum>(glCheckFramebufferStatus( static_cast<GLenum>(target) )); }
void lsOpenGL::DrawBuffer( lsEnum mode )                                                    { glDrawBuffer( static_cast<GLenum>(mode) ); }
void lsOpenGL::ReadBuffer( lsEnum mode )                                                    { glReadBuffer( static_cast<GLenum>(mode) ); }
void lsOpenGL::ReadPixels( lsInt x, lsInt y, lsSizei width, lsSizei height,
                           lsEnum format, lsEnum type, void* data )                        { glReadPixels( static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLenum>(format), static_cast<GLenum>(type), data ); }

// ---- Buffer / VAO objects ----
void lsOpenGL::GenBuffers( lsSizei n, lsBufferID* buffers )                                { glGenBuffers( static_cast<GLsizei>(n), reinterpret_cast<GLuint*>(buffers) ); }
void lsOpenGL::DeleteBuffers( lsSizei n, lsBufferID* buffers )                             { glDeleteBuffers( static_cast<GLsizei>(n), reinterpret_cast<GLuint*>(buffers) ); }
void lsOpenGL::BindBuffer( lsEnum target, lsBufferID buffer )                              { glBindBuffer( static_cast<GLenum>(target), static_cast<GLuint>(buffer) ); }
void lsOpenGL::BufferData( lsEnum target, lsSizeiptr size,
                           const void* data, lsEnum usage )                                { glBufferData( static_cast<GLenum>(target), static_cast<GLsizeiptr>(size), data, static_cast<GLenum>(usage) ); }
void lsOpenGL::GenVertexArrays( lsSizei n, lsVAOID* arrays )                                { glGenVertexArrays( static_cast<GLsizei>(n), reinterpret_cast<GLuint*>(arrays) ); }
void lsOpenGL::DeleteVertexArrays( lsSizei n, lsVAOID* arrays )                             { glDeleteVertexArrays( static_cast<GLsizei>(n), reinterpret_cast<GLuint*>(arrays) ); }
void lsOpenGL::BindVertexArray( lsVAOID vao )                                               { glBindVertexArray( static_cast<GLuint>(vao) ); }
void lsOpenGL::EnableVertexAttribArray( lsUInt index )                                       { glEnableVertexAttribArray( static_cast<GLuint>(index) ); }
void lsOpenGL::VertexAttribPointer( lsUInt index, lsInt size, lsEnum type,
                                    lsBoolean normalized, lsSizei stride,
                                    const void* pointer )                                  { glVertexAttribPointer( static_cast<GLuint>(index), static_cast<GLint>(size), static_cast<GLenum>(type), static_cast<GLboolean>(normalized), static_cast<GLsizei>(stride), pointer ); }

// ---- Draw calls ----
void lsOpenGL::DrawArrays( lsEnum mode, lsInt first, lsSizei count )                       { glDrawArrays( static_cast<GLenum>(mode), static_cast<GLint>(first), static_cast<GLsizei>(count) ); }
void lsOpenGL::DrawElements( lsEnum mode, lsSizei count, lsEnum type, const void* indices ) { glDrawElements( static_cast<GLenum>(mode), static_cast<GLsizei>(count), static_cast<GLenum>(type), indices ); }

// ---- Shader / program objects ----
lsShaderID lsOpenGL::CreateShader( lsEnum type )                                            { return static_cast<lsShaderID>(glCreateShader( static_cast<GLenum>(type) )); }
lsProgramID lsOpenGL::CreateProgram()                                                       { return static_cast<lsProgramID>(glCreateProgram()); }
void lsOpenGL::ShaderSource( lsShaderID shader, lsSizei count,
                             const lsChar** string, const lsInt* length )                 { glShaderSource( static_cast<GLuint>(shader), static_cast<GLsizei>(count), reinterpret_cast<const GLchar**>(string), reinterpret_cast<const GLint*>(length) ); }
void lsOpenGL::CompileShader( lsShaderID shader )                                           { glCompileShader( static_cast<GLuint>(shader) ); }
void lsOpenGL::AttachShader( lsProgramID program, lsShaderID shader )                       { glAttachShader( static_cast<GLuint>(program), static_cast<GLuint>(shader) ); }
void lsOpenGL::DetachShader( lsProgramID program, lsShaderID shader )                       { glDetachShader( static_cast<GLuint>(program), static_cast<GLuint>(shader) ); }
void lsOpenGL::LinkProgram( lsProgramID program )                                           { glLinkProgram( static_cast<GLuint>(program) ); }
void lsOpenGL::UseProgram( lsProgramID program )                                            { glUseProgram( static_cast<GLuint>(program) ); }
void lsOpenGL::DeleteShader( lsShaderID shader )                                            { glDeleteShader( static_cast<GLuint>(shader) ); }
void lsOpenGL::DeleteProgram( lsProgramID program )                                         { glDeleteProgram( static_cast<GLuint>(program) ); }
lsUniformID lsOpenGL::GetUniformLocation( lsProgramID program, const lsChar* name )        { return static_cast<lsUniformID>(glGetUniformLocation( static_cast<GLuint>(program), reinterpret_cast<const GLchar*>(name) )); }
void lsOpenGL::GetShaderiv( lsShaderID shader, lsEnum pname, lsInt* params )                { glGetShaderiv( static_cast<GLuint>(shader), static_cast<GLenum>(pname), params ); }
void lsOpenGL::GetShaderInfoLog( lsShaderID shader, lsSizei bufSize,
                                 lsSizei* length, lsChar* infoLog )                        { glGetShaderInfoLog( static_cast<GLuint>(shader), static_cast<GLsizei>(bufSize), reinterpret_cast<GLsizei*>(length), reinterpret_cast<GLchar*>(infoLog) ); }
void lsOpenGL::GetProgramiv( lsProgramID program, lsEnum pname, lsInt* params )             { glGetProgramiv( static_cast<GLuint>(program), static_cast<GLenum>(pname), params ); }
void lsOpenGL::GetProgramInfoLog( lsProgramID program, lsSizei bufSize,
                                  lsSizei* length, lsChar* infoLog )                       { glGetProgramInfoLog( static_cast<GLuint>(program), static_cast<GLsizei>(bufSize), reinterpret_cast<GLsizei*>(length), reinterpret_cast<GLchar*>(infoLog) ); }

// ---- Uniform setters ----
void lsOpenGL::Uniform1i( lsUniformID location, lsInt v0 )                                  { glUniform1i( static_cast<GLint>(location), static_cast<GLint>(v0) ); }
void lsOpenGL::Uniform1f( lsUniformID location, lsFloat v0 )                                { glUniform1f( static_cast<GLint>(location), static_cast<GLfloat>(v0) ); }
void lsOpenGL::Uniform3f( lsUniformID location, lsFloat v0, lsFloat v1, lsFloat v2 )        { glUniform3f( static_cast<GLint>(location), static_cast<GLfloat>(v0), static_cast<GLfloat>(v1), static_cast<GLfloat>(v2) ); }
void lsOpenGL::Uniform4f( lsUniformID location, lsFloat v0, lsFloat v1, lsFloat v2, lsFloat v3 ) { glUniform4f( static_cast<GLint>(location), static_cast<GLfloat>(v0), static_cast<GLfloat>(v1), static_cast<GLfloat>(v2), static_cast<GLfloat>(v3) ); }
void lsOpenGL::UniformMatrix3fv( lsUniformID location, lsSizei count,
                                 lsBoolean transpose, const lsFloat* value )              { glUniformMatrix3fv( static_cast<GLint>(location), static_cast<GLsizei>(count), static_cast<GLboolean>(transpose), reinterpret_cast<const GLfloat*>(value) ); }
void lsOpenGL::UniformMatrix4fv( lsUniformID location, lsSizei count,
                                 lsBoolean transpose, const lsFloat* value )              { glUniformMatrix4fv( static_cast<GLint>(location), static_cast<GLsizei>(count), static_cast<GLboolean>(transpose), reinterpret_cast<const GLfloat*>(value) ); }

// ---- Pipeline / rasterizer state ----
void lsOpenGL::Enable( lsEnum cap )                                                         { glEnable( static_cast<GLenum>(cap) ); }
void lsOpenGL::Disable( lsEnum cap )                                                        { glDisable( static_cast<GLenum>(cap) ); }
void lsOpenGL::CullFace( lsEnum mode )                                                      { glCullFace( static_cast<GLenum>(mode) ); }
void lsOpenGL::BlendFunc( lsEnum sfactor, lsEnum dfactor )                                  { glBlendFunc( static_cast<GLenum>(sfactor), static_cast<GLenum>(dfactor) ); }
void lsOpenGL::DepthFunc( lsEnum func )                                                     { glDepthFunc( static_cast<GLenum>(func) ); }
void lsOpenGL::DepthMask( lsBoolean flag )                                                  { glDepthMask( static_cast<GLboolean>(flag) ); }
void lsOpenGL::DepthRangef( lsFloat near, lsFloat far )                                     { glDepthRangef( static_cast<GLfloat>(near), static_cast<GLfloat>(far) ); }
void lsOpenGL::ColorMask( lsBoolean r, lsBoolean g, lsBoolean b, lsBoolean a )              { glColorMask( static_cast<GLboolean>(r), static_cast<GLboolean>(g), static_cast<GLboolean>(b), static_cast<GLboolean>(a) ); }
void lsOpenGL::PolygonOffset( lsFloat factor, lsFloat units )                               { glPolygonOffset( static_cast<GLfloat>(factor), static_cast<GLfloat>(units) ); }
void lsOpenGL::ShadeModel( lsEnum mode )                                                    { glShadeModel( static_cast<GLenum>(mode) ); }
void lsOpenGL::Viewport( lsInt x, lsInt y, lsSizei w, lsSizei h )                           { glViewport( static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(w), static_cast<GLsizei>(h) ); }

// ---- Buffer clear ----
void lsOpenGL::Clear( lsBitfield mask )                                                     { glClear( static_cast<GLbitfield>(mask) ); }
void lsOpenGL::ClearColor( lsFloat r, lsFloat g, lsFloat b, lsFloat a )                    { glClearColor( static_cast<GLfloat>(r), static_cast<GLfloat>(g), static_cast<GLfloat>(b), static_cast<GLfloat>(a) ); }
void lsOpenGL::ClearDepth( lsDouble d )                                                     { glClearDepth( static_cast<GLdouble>(d) ); }

// ---- Occlusion / timer queries ----
void lsOpenGL::GenQueries( lsSizei n, lsQueryID* ids )                                     { glGenQueries( static_cast<GLsizei>(n), reinterpret_cast<GLuint*>(ids) ); }
void lsOpenGL::DeleteQueries( lsSizei n, lsQueryID* ids )                                  { glDeleteQueries( static_cast<GLsizei>(n), reinterpret_cast<GLuint*>(ids) ); }
void lsOpenGL::BeginQuery( lsEnum target, lsQueryID id )                                    { glBeginQuery( static_cast<GLenum>(target), static_cast<GLuint>(id) ); }
void lsOpenGL::EndQuery( lsEnum target )                                                    { glEndQuery( static_cast<GLenum>(target) ); }
void lsOpenGL::GetQueryObjectiv( lsQueryID id, lsEnum pname, lsInt* params )               { glGetQueryObjectiv( static_cast<GLuint>(id), static_cast<GLenum>(pname), params ); }

// ---- Diagnostic getters ----
lsEnum lsOpenGL::GetError()                                                                 { return static_cast<lsEnum>(glGetError()); }
void lsOpenGL::GetIntegerv( lsEnum pname, lsInt* params )                                   { glGetIntegerv( static_cast<GLenum>(pname), params ); }
