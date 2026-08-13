/**
 * @file lsVulkanShader.cpp
 * @brief GLSL transform + glslang compile for the Vulkan backend.
 */

#include "lsVulkanShader.h"

#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <iostream>
#include <regex>
#include <sstream>

using namespace std;

namespace
{
	bool isOpaqueType( const string& t )
	{
		return t.rfind( "sampler", 0 ) == 0 || t.rfind( "image", 0 ) == 0 ||
			   t.rfind( "texture", 0 ) == 0 || t.rfind( "usampler", 0 ) == 0 ||
			   t.rfind( "isampler", 0 ) == 0; // L5: integer samplers (cluster grid)
	}

	uint32_t alignOf( const string& t )
	{
		if( t == "vec2" ) return 8;
		if( t == "vec3" || t == "vec4" ) return 16;
		if( t == "mat3" || t == "mat4" ) return 16;
		return 4;
	}
	uint32_t sizeOf( const string& t )
	{
		if( t == "vec2" ) return 8;
		if( t == "vec3" ) return 12;
		if( t == "vec4" ) return 16;
		if( t == "mat3" ) return 48;	// 3 columns x 16B stride
		if( t == "mat4" ) return 64;
		return 4;
	}
	uint32_t alignUp( uint32_t v, uint32_t a )
	{
		return ( v + a - 1 ) & ~( a - 1 );
	}

	bool g_compilerReady = false;
}

// ---------------------------------------------------------------------
// parse
// ---------------------------------------------------------------------
lsVkStageParse lsVkShader::parse( const string& source )
{
	lsVkStageParse out;

	std::regex uniformRe(
		R"(^\s*uniform\s+([A-Za-z_]\w*)\s+([A-Za-z_]\w*)\s*(?:\[\s*(\d+)\s*\])?\s*;)" );

	std::vector<string> bodyLines;
	std::stringstream ss( source );
	string line;
	while( std::getline( ss, line ) )
	{
		std::smatch m;
		// global in/out decls WITHOUT a layout qualifier need explicit
		// locations for Vulkan (assigned later by assignInterface)
		static const std::regex ioRe(
			R"(^\s*(?:flat\s+|smooth\s+|noperspective\s+|centroid\s+)?(in|out)\s+([A-Za-z_]\w*)\s+([A-Za-z_]\w*)\s*(?:\[\s*(\d+)\s*\])?\s*;)" );
		if( std::regex_search( line, m, ioRe ) )
		{
			lsVkInterfaceVar v;
			v.name = m[3];
			v.arraySize = m[4].matched ? (uint32_t)stoul( m[4] ) : 0;
			( m[1] == "in" ? out.ins : out.outs ).push_back( v );
			bodyLines.push_back( line );	// assemble() injects the layout
			continue;
		}
		// regex_search: pattern is ^-anchored but tolerates trailing
		// comments (uniform mat4 mMatrix; // Model Matrix)
		if( std::regex_search( line, m, uniformRe ) )
		{
			const string type = m[1];
			const string name = m[2];
			const uint32_t arr = m[3].matched ? (uint32_t)stoul( m[3] ) : 0;

			if( isOpaqueType( type ) )
			{
				out.samplers.push_back( name );
				out.samplerTypes.push_back( type );
				out.samplerArrays.push_back( arr );
				continue;
			}

			lsVkUniformMember mem;
			mem.name = name;
			mem.glslType = type;
			mem.arraySize = arr;
			mem.size = sizeOf( type ) * ( arr ? arr : 1 );
			out.members.push_back( mem );
			continue;
		}
		bodyLines.push_back( line );
	}

	string body;
	for( auto& l : bodyLines )
	{
		body += l;
		body += '\n';
	}
	for( const auto& mem : out.members )
	{
		body = std::regex_replace( body,
			std::regex( "\\b" + mem.name + "\\b" ), "_ubo." + mem.name );
	}
	out.body = body;
	return out;
}

// ---------------------------------------------------------------------
// mergeLayouts
// ---------------------------------------------------------------------
lsVkProgramLayout lsVkShader::mergeLayouts( const lsVkStageParse& vs,
											const lsVkStageParse& fs )
{
	lsVkProgramLayout layout;

	auto addMembers = [&]( const std::vector<lsVkUniformMember>& src )
	{
		for( const auto& mem : src )
		{
			bool found = false;
			for( auto& existing : layout.members )
				if( existing.name == mem.name ) { found = true; break; }
			if(! found ) layout.members.push_back( mem );
		}
	};
	addMembers( vs.members );
	addMembers( fs.members );

	// std140 offsets on the canonical list
	uint32_t cursor = 0;
	for( auto& mem : layout.members )
	{
		// std140: arrays are ALWAYS vec4-aligned, regardless of element type
		cursor = alignUp( cursor, mem.arraySize ? 16 : alignOf( mem.glslType ) );
		mem.offset = cursor;
		const uint32_t stride = mem.arraySize
			? alignUp( sizeOf( mem.glslType ), 16 ) : sizeOf( mem.glslType );
		mem.elemStride = stride;	// for [i] element location decoding
		cursor += stride * ( mem.arraySize ? mem.arraySize : 1 );
	}
	layout.blockSize = alignUp( cursor, 16 );

	auto addSamplers = [&]( const std::vector<std::string>& src,
							const std::vector<uint32_t>& arrs )
	{
		for( size_t i = 0; i < src.size(); i++ )
		{
			bool found = false;
			for( auto& e : layout.samplers )
				if( e == src[i] ) { found = true; break; }
			if(! found )
			{
				layout.samplers.push_back( src[i] );
				layout.samplerArrays.push_back( i < arrs.size() ? arrs[i] : 0 );
			}
		}
	};
	addSamplers( vs.samplers, vs.samplerArrays );
	addSamplers( fs.samplers, fs.samplerArrays );

	return layout;
}

// ---------------------------------------------------------------------
// assignInterface
// ---------------------------------------------------------------------
void lsVkShader::assignInterface( lsVkStageParse& vs, lsVkStageParse& fs )
{
	// VS outs: sequential; arrays consume consecutive locations
	uint32_t next = 0;
	for( auto& v : vs.outs )
	{
		v.location = next;
		next += v.arraySize ? v.arraySize : 1;
	}
	// FS ins: match VS outs by name; unmatched take the next free slot
	for( auto& v : fs.ins )
	{
		uint32_t loc = UINT32_MAX;
		for( const auto& o : vs.outs )
			if( o.name == v.name ) { loc = o.location; break; }
		if( loc == UINT32_MAX )
		{
			loc = next;
			next += v.arraySize ? v.arraySize : 1;
		}
		v.location = loc;
	}
	// FS outs: colour attachments, numbered from 0
	next = 0;
	for( auto& v : fs.outs )
	{
		v.location = next;
		next += v.arraySize ? v.arraySize : 1;
	}
	// VS ins without explicit layout: declaration order (engine shaders
	// carry explicit layout(location=N) on VS ins, so this is a fallback)
	next = 0;
	for( auto& v : vs.ins )
	{
		v.location = next;
		next += v.arraySize ? v.arraySize : 1;
	}
}

// ---------------------------------------------------------------------
// assemble
// ---------------------------------------------------------------------
string lsVkShader::assemble( const lsVkStageParse& stage,
							 const lsVkProgramLayout& layout )
{
	std::stringstream out;
	std::regex versionRe( R"(^\s*#version)" );

	std::stringstream bodyStream( stage.body );
	string line;
	bool versionWritten = false;
	static const std::regex ioRe(
		R"(^\s*(?:flat\s+|smooth\s+|noperspective\s+|centroid\s+)?(in|out)\s+([A-Za-z_]\w*)\s+([A-Za-z_]\w*)\s*(?:\[\s*\d+\s*\])?\s*;)" );
	auto findLoc = [&]( const std::string& dir, const std::string& name,
						uint32_t& loc ) -> bool
	{
		const auto& vec = ( dir == "in" ) ? stage.ins : stage.outs;
		for( const auto& v : vec )
			if( v.name == name ) { loc = v.location; return true; }
		return false;
	};
	while( std::getline( bodyStream, line ) )
	{
		std::smatch im;
		uint32_t loc = 0;
		if( std::regex_search( line, im, ioRe ) &&
			findLoc( im[1], im[3], loc ) )
		{
			out << "layout(location = " << loc << ") " << line << '\n';
			continue;
		}
		if(! versionWritten && std::regex_search( line, versionRe ) )
		{
			out << "#version 450\n";
			if(! layout.members.empty() )
			{
				out << "layout(set=0, binding=0, std140) uniform DefaultUniforms {\n";
				for( const auto& mem : layout.members )
				{
					out << "    " << mem.glslType << " " << mem.name;
					if( mem.arraySize ) out << "[" << mem.arraySize << "]";
					out << ";\n";
				}
				out << "} _ubo;\n";
			}
			for( size_t i = 0; i < stage.samplers.size(); i++ )
			{
				// binding comes from the MERGED sampler list
				uint32_t binding = 1;
				for( size_t k = 0; k < layout.samplers.size(); k++ )
					if( layout.samplers[k] == stage.samplers[i] )
						binding = 1 + (uint32_t)k;
				out << "layout(set=0, binding=" << binding << ") uniform "
					<< stage.samplerTypes[i] << " " << stage.samplers[i];
				if( i < stage.samplerArrays.size() && stage.samplerArrays[i] > 0 )
					out << "[" << stage.samplerArrays[i] << "]";
				out << ";\n";
			}
			versionWritten = true;
			continue;
		}
		out << line << '\n';
	}
	if(! versionWritten )
		return "#version 450\n" + out.str();
	return out.str();
}

// ---------------------------------------------------------------------
// compile
// ---------------------------------------------------------------------
void lsVkShader::initCompiler()
{
	if(! g_compilerReady )
	{
		glslang::InitializeProcess();
		g_compilerReady = true;
	}
}

void lsVkShader::shutdownCompiler()
{
	if( g_compilerReady )
	{
		glslang::FinalizeProcess();
		g_compilerReady = false;
	}
}

vector<uint32_t> lsVkShader::compile( const string& source, int stage, string& log )
{
	initCompiler();

	const EShLanguage lang = ( stage == 0 ) ? EShLangVertex : EShLangFragment;

	glslang::TShader shader( lang );
	const char* src = source.c_str();
	shader.setStrings( &src, 1 );
	shader.setEnvInput( glslang::EShSourceGlsl, lang, glslang::EShClientVulkan, 100 );
	shader.setEnvClient( glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2 );
	shader.setEnvTarget( glslang::EShTargetSpv, glslang::EShTargetSpv_1_0 );
	shader.setAutoMapLocations( true );
	shader.setAutoMapBindings( true );

	const TBuiltInResource resources = *GetDefaultResources();
	const EShMessages messages = (EShMessages)( EShMsgSpvRules | EShMsgVulkanRules );

	if(! shader.parse( &resources, 450, false, messages ) )
	{
		log = shader.getInfoLog();
		return {};
	}

	glslang::TProgram program;
	program.addShader( &shader );
	if(! program.link( messages ) )
	{
		log = program.getInfoLog();
		return {};
	}

	vector<uint32_t> spirv;
	glslang::GlslangToSpv( *program.getIntermediate( lang ), spirv );
	return spirv;
}
