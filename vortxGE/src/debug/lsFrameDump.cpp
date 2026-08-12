/**
 * @file lsFrameDump.cpp
 * @brief D2 (Plan.md): per-draw frame state dump — implementation.
 *
 * The ENTIRE translation unit is compiled out when VORTX_DEBUG_TOOLS
 * is undefined (Release): empty object file, no strings, no code.
 * See lsFrameDump.h for the design notes.
 */

#include "lsFrameDump.h"

#if defined(VORTX_DEBUG_TOOLS)

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>

namespace lsFrameDump
{

// GL numeric constants (stable ABI values; re-declared here so this
// module does not include lsRenderConstants.h / <GL/glew.h>)
static constexpr uint32_t GLC_DEPTH_COMPONENT	= 0x1902;
static constexpr uint32_t GLC_RED				= 0x1903;
static constexpr uint32_t GLC_RGB				= 0x1907;
static constexpr uint32_t GLC_RGBA				= 0x1908;
static constexpr uint32_t GLC_RG				= 0x8227;
static constexpr uint32_t GLC_DEPTH_STENCIL		= 0x84F9;
static constexpr uint32_t GLC_UNSIGNED_BYTE		= 0x1401;
static constexpr uint32_t GLC_UNSIGNED_SHORT	= 0x1403;
static constexpr uint32_t GLC_UNSIGNED_INT		= 0x1405;
static constexpr uint32_t GLC_FLOAT				= 0x1406;
static constexpr uint32_t GLC_DEPTH_TEST		= 0x0B71;
static constexpr uint32_t GLC_CULL_FACE			= 0x0B44;
static constexpr uint32_t GLC_BLEND				= 0x0BE2;

// --------------------------------------------------------------------
// Env parsing / frame arming
// --------------------------------------------------------------------
static bool			s_parsed = false;
static bool			s_armed = false;
static int			s_target = 30;
static std::string	s_path;
static int			s_frame = 0;		// completed frames
static std::string	s_backend = "unknown";

static void parseEnv()
{
	if( s_parsed ) return;
	s_parsed = true;
	const char* e = getenv( "VORTX_FRAME_DUMP" );
	if(! e || ! *e ) return;
	const std::string v = e;
	// "N:path.json" or bare "path.json" (default frame 30)
	const size_t colon = v.find( ':' );
	if( colon != std::string::npos && colon > 0 &&
		v.find_first_not_of( "0123456789" ) == colon )
	{
		s_target = atoi( v.substr( 0, colon ).c_str() );
		s_path = v.substr( colon + 1 );
	}
	else
		s_path = v;
	if( s_path.empty() || s_target < 1 ) { s_path.clear(); return; }
	s_armed = true;
	std::cout << "VORTX_FRAME_DUMP: armed for frame " << s_target
			  << " -> " << s_path << std::endl;
}

void setBackend( const char* name ) { if( name ) s_backend = name; }

bool recording()
{
	parseEnv();
	return s_armed && s_frame == s_target - 1;
}

// --------------------------------------------------------------------
// Identity signatures
// --------------------------------------------------------------------
static uint64_t fnv1a( const void* data, size_t bytes )
{
	const uint8_t* p = (const uint8_t*)data;
	uint64_t h = 14695981039346656037ull;
	for( size_t i = 0; i < bytes; i++ )
	{ h ^= p[i]; h *= 1099511628211ull; }
	return h;
}

// programs
static std::map<uint32_t, uint64_t>				s_shaderHash;
static std::map<uint32_t, std::vector<uint32_t>>	s_progShaders;
static std::map<uint32_t, uint64_t>				s_progSig;

void trackShaderSource( uint32_t shader, const std::string& src )
{
	parseEnv();
	if(! s_armed ) return;
	s_shaderHash[shader] = fnv1a( src.data(), src.size() );
}

void trackAttach( uint32_t program, uint32_t shader )
{
	if(! s_armed ) return;
	s_progShaders[program].push_back( shader );
}

void trackLink( uint32_t program )
{
	if(! s_armed ) return;
	uint64_t h = 14695981039346656037ull;
	for( uint32_t sh : s_progShaders[program] )
	{
		const uint64_t sh_h = s_shaderHash[sh];
		for( int i = 0; i < 8; i++ )
		{ h ^= ( sh_h >> ( i * 8 ) ) & 0xFF; h *= 1099511628211ull; }
	}
	s_progSig[program] = h;
}

// textures
struct TexInfo { int w = 0, h = 0, fmt = 0; uint64_t hash = 0; bool set = false; };
static std::map<uint32_t, TexInfo> s_texInfo;

static size_t pixelBytes( int w, int h, int format, int type )
{
	int ch = 4;
	if( format == (int)GLC_RED || format == (int)GLC_DEPTH_COMPONENT ) ch = 1;
	else if( format == (int)GLC_RG || format == (int)GLC_DEPTH_STENCIL ) ch = 2;
	else if( format == (int)GLC_RGB ) ch = 3;
	int bpc = 1;
	if( type == (int)GLC_FLOAT || type == (int)GLC_UNSIGNED_INT ) bpc = 4;
	else if( type == (int)GLC_UNSIGNED_SHORT ) bpc = 2;
	return (size_t)w * (size_t)h * (size_t)( ch * bpc );
}

void trackTexImage( uint32_t tex, int level, int internalFormat,
					int w, int h, int format, int type, const void* data )
{
	parseEnv();
	if(! s_armed || level != 0 || tex == 0 ) return;
	TexInfo ti;
	ti.w = w; ti.h = h; ti.fmt = internalFormat;
	ti.hash = data ? fnv1a( data, pixelBytes( w, h, format, type ) ) : 0;
	ti.set = true;
	s_texInfo[tex] = ti;
}

// --------------------------------------------------------------------
// GL-side tracked state
// --------------------------------------------------------------------
static uint32_t	s_curProg = 0, s_curVAO = 0, s_curFBO = 0;
static int		s_vp[4] = { 0, 0, 0, 0 };
static uint32_t	s_activeUnit = 0;
static uint32_t	s_unitTex[64] = {};
// defaults mirror lsVulkan's GL-convention state cache so both
// backends report the same values for never-touched state
static bool		s_depthTest = false, s_depthWrite = true;
static uint32_t	s_depthFunc = 0x0203;		// LS_LEQUAL
static bool		s_cull = false;
static uint32_t	s_cullFace = 0x0405;		// LS_BACK
static bool		s_blend = false;
static uint32_t	s_blendSrc = 0x0302, s_blendDst = 0x0303;

static std::map<uint32_t, std::map<int, std::string>>					s_uniNames;
static std::map<uint32_t, std::map<std::string, std::vector<float>>>	s_u;
static std::map<uint32_t, std::map<std::string, std::vector<int>>>		s_iu;

void trackTexImageBound( int level, int internalFormat, int w, int h,
						 int format, int type, const void* data )
{
	parseEnv();
	if(! s_armed ) return;
	trackTexImage( s_unitTex[s_activeUnit], level, internalFormat,
				   w, h, format, type, data );
}

void trackUniformName( uint32_t program, int location, const char* name )
{
	if(! s_armed || location < 0 || ! name ) return;
	s_uniNames[program][location] = name;
}
void trackUseProgram( uint32_t program )	{ s_curProg = program; }
void trackBindVAO( uint32_t vao )			{ s_curVAO = vao; }
void trackBindFBO( uint32_t fbo )			{ s_curFBO = fbo; }
void trackViewport( int x, int y, int w, int h )
{ s_vp[0] = x; s_vp[1] = y; s_vp[2] = w; s_vp[3] = h; }
void trackActiveTexture( uint32_t unit )	{ if( unit < 64 ) s_activeUnit = unit; }
void trackBindTexture( uint32_t tex )		{ s_unitTex[s_activeUnit] = tex; }

void trackUniformF( int location, const float* v, int n )
{
	if(! s_armed || ! v || n <= 0 ) return;
	auto pit = s_uniNames.find( s_curProg );
	if( pit == s_uniNames.end() ) return;
	auto nit = pit->second.find( location );
	if( nit == pit->second.end() ) return;
	s_u[s_curProg][nit->second].assign( v, v + n );
}

void trackUniformI( int location, const int* v, int n )
{
	if(! s_armed || ! v || n <= 0 ) return;
	auto pit = s_uniNames.find( s_curProg );
	if( pit == s_uniNames.end() ) return;
	auto nit = pit->second.find( location );
	if( nit == pit->second.end() ) return;
	s_iu[s_curProg][nit->second].assign( v, v + n );
}

void trackEnable( uint32_t cap, bool on )
{
	if( cap == GLC_DEPTH_TEST ) s_depthTest = on;
	else if( cap == GLC_CULL_FACE ) s_cull = on;
	else if( cap == GLC_BLEND ) s_blend = on;
}
void trackCullFace( uint32_t mode )					{ s_cullFace = mode; }
void trackBlendFunc( uint32_t src, uint32_t dst )	{ s_blendSrc = src; s_blendDst = dst; }
void trackDepthFunc( uint32_t func )				{ s_depthFunc = func; }
void trackDepthMask( bool on )						{ s_depthWrite = on; }

// --------------------------------------------------------------------
// Draw records
// --------------------------------------------------------------------
static std::vector<DrawRec> s_draws;

DrawRec* openDraw()
{
	if(! recording() ) return nullptr;
	s_draws.emplace_back();
	return &s_draws.back();
}

void commitDraw() { /* record already in s_draws via openDraw */ }

void glRecordDraw( uint32_t mode, uint32_t count, uint32_t first,
				   bool indexed )
{
	DrawRec* r = openDraw();
	if(! r ) return;
	r->fbo = s_curFBO; r->prog = s_curProg; r->vao = s_curVAO;
	r->mode = mode; r->count = count; r->first = first;
	r->indexed = indexed;
	for( int i = 0; i < 4; i++ ) r->vp[i] = s_vp[i];
	r->depthTest = s_depthTest; r->depthWrite = s_depthWrite;
	r->depthFunc = s_depthFunc;
	r->cull = s_cull; r->cullFace = s_cullFace;
	r->blend = s_blend; r->blendSrc = s_blendSrc; r->blendDst = s_blendDst;
	for( uint32_t u = 0; u < 64; u++ )
		if( s_unitTex[u] ) r->tex[u] = s_unitTex[u];
	auto uit = s_u.find( s_curProg );
	if( uit != s_u.end() ) r->u = uit->second;
	auto iit = s_iu.find( s_curProg );
	if( iit != s_iu.end() ) r->iu = iit->second;
}

// --------------------------------------------------------------------
// JSON serialisation
// --------------------------------------------------------------------
static void jsonFloats( std::ostream& os, const std::vector<float>& v )
{
	os << '[';
	char buf[32];
	for( size_t i = 0; i < v.size(); i++ )
	{
		snprintf( buf, sizeof( buf ), "%.7g", (double)v[i] );
		os << ( i ? "," : "" ) << buf;
	}
	os << ']';
}

static void jsonInts( std::ostream& os, const std::vector<int>& v )
{
	os << '[';
	for( size_t i = 0; i < v.size(); i++ )
		os << ( i ? "," : "" ) << v[i];
	os << ']';
}

static void writeFile()
{
	std::ofstream os( s_path, std::ios::out | std::ios::trunc );
	if(! os )
	{
		std::cout << "VORTX_FRAME_DUMP: ERROR cannot open " << s_path << std::endl;
		return;
	}
	os << "{\"tool\":\"vortxGE-frame-dump\",\"version\":1,"
	   << "\"backend\":\"" << s_backend << "\",\"frame\":" << s_target << ",";

	os << "\"programs\":{";
	bool first = true;
	char hexbuf[24];
	for( const auto& [id, sig] : s_progSig )
	{
		snprintf( hexbuf, sizeof( hexbuf ), "%016llx", (unsigned long long)sig );
		os << ( first ? "" : "," ) << "\"" << id << "\":{\"sig\":\"" << hexbuf << "\"}";
		first = false;
	}
	os << "},\"textures\":{";
	first = true;
	for( const auto& [id, ti] : s_texInfo )
	{
		snprintf( hexbuf, sizeof( hexbuf ), "%016llx", (unsigned long long)ti.hash );
		os << ( first ? "" : "," ) << "\"" << id << "\":{\"w\":" << ti.w
		   << ",\"h\":" << ti.h << ",\"fmt\":" << ti.fmt
		   << ",\"hash\":\"" << hexbuf << "\"}";
		first = false;
	}
	os << "},\"draws\":[";
	for( size_t d = 0; d < s_draws.size(); d++ )
	{
		const DrawRec& r = s_draws[d];
		os << ( d ? "," : "" ) << "{\"seq\":" << d
		   << ",\"fbo\":" << r.fbo << ",\"prog\":" << r.prog
		   << ",\"vao\":" << r.vao << ",\"mode\":" << r.mode
		   << ",\"indexed\":" << ( r.indexed ? 1 : 0 )
		   << ",\"count\":" << r.count << ",\"first\":" << r.first
		   << ",\"vp\":[" << r.vp[0] << "," << r.vp[1] << ","
		   << r.vp[2] << "," << r.vp[3] << "]"
		   << ",\"st\":{\"dt\":" << ( r.depthTest ? 1 : 0 )
		   << ",\"dw\":" << ( r.depthWrite ? 1 : 0 )
		   << ",\"df\":" << r.depthFunc
		   << ",\"cull\":" << ( r.cull ? 1 : 0 )
		   << ",\"cf\":" << r.cullFace
		   << ",\"blend\":" << ( r.blend ? 1 : 0 )
		   << ",\"bs\":" << r.blendSrc << ",\"bd\":" << r.blendDst << "}";
		os << ",\"tex\":{";
		first = true;
		for( const auto& [unit, tex] : r.tex )
		{ os << ( first ? "" : "," ) << "\"" << unit << "\":" << tex; first = false; }
		os << "},\"u\":{";
		first = true;
		for( const auto& [name, v] : r.u )
		{ os << ( first ? "" : "," ) << "\"" << name << "\":"; jsonFloats( os, v ); first = false; }
		os << "},\"iu\":{";
		first = true;
		for( const auto& [name, v] : r.iu )
		{ os << ( first ? "" : "," ) << "\"" << name << "\":"; jsonInts( os, v ); first = false; }
		os << "}}";
	}
	os << "]}\n";
	os.close();
	std::cout << "VORTX_FRAME_DUMP: wrote " << s_path << " ("
			  << s_draws.size() << " draws, frame " << s_target << ")"
			  << std::endl;
}

void frameTick()
{
	parseEnv();
	if( s_armed && s_frame == s_target - 1 )
	{
		writeFile();
		s_armed = false;	// one-shot
	}
	s_frame++;
}

} // namespace lsFrameDump

#endif // VORTX_DEBUG_TOOLS
