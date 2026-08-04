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
			   t.rfind( "texture", 0 ) == 0;
	}

	// std140 base alignment + packed size for the engine's uniform types
	uint32_t alignOf( const string& t )
	{
		if( t == "vec2" ) return 8;
		if( t == "vec3" || t == "vec4" ) return 16;
		if( t == "mat3" || t == "mat4" ) return 16;
		return 4;	// int/uint/float/bool + ivec/etc (engine doesn't use them yet)
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
// transform
// ---------------------------------------------------------------------
lsVkShaderTransform lsVkShader::transform( const string& source )
{
	lsVkShaderTransform out;

	// Split into lines, classify uniform declarations
	std::regex uniformRe(
		R"(^\s*uniform\s+([A-Za-z_]\w*)\s+([A-Za-z_]\w*)\s*(?:\[\s*(\d+)\s*\])?\s*;)" );
	std::regex versionRe( R"(^\s*#version)" );

	std::vector<string> bodyLines;
	std::stringstream ss( source );
	string line;
	while( std::getline( ss, line ) )
	{
		std::smatch m;
		if( std::regex_match( line, m, uniformRe ) )
		{
			const string type = m[1];
			const string name = m[2];
			const uint32_t arr = m[3].matched ? (uint32_t)stoul( m[3] ) : 0;

			if( isOpaqueType( type ) )
			{
				out.samplers.push_back( name );
				// declaration rewritten below (binding = 1 + index)
				continue;
			}

			lsVkUniformMember mem;
			mem.name = name;
			mem.glslType = type;
			mem.arraySize = arr;
			mem.size = sizeOf( type ) * ( arr ? arr : 1 );
			out.members.push_back( mem );
			continue;	// declaration moves into the block
		}
		bodyLines.push_back( line );
	}

	// std140 offsets
	uint32_t cursor = 0;
	for( auto& mem : out.members )
	{
		const uint32_t a = alignOf( mem.glslType );
		cursor = alignUp( cursor, a );
		mem.offset = cursor;
		// arrays: each element occupies a 16-byte-aligned slot
		const uint32_t stride = mem.arraySize ? alignUp( sizeOf( mem.glslType ), 16 )
											  : sizeOf( mem.glslType );
		cursor += stride * ( mem.arraySize ? mem.arraySize : 1 );
	}
	out.blockSize = alignUp( cursor, 16 );

	// Rewrite usages (before the block is injected so its declarations
	// are not touched)
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

	// Reassemble: #version 450 + block + sampler decls + body
	std::stringstream rebuilt;
	bool versionWritten = false;
	std::stringstream bodyStream( body );
	while( std::getline( bodyStream, line ) )
	{
		if(! versionWritten && std::regex_search( line, versionRe ) )
		{
			rebuilt << "#version 450\n";
			if(! out.members.empty() )
			{
				rebuilt << "layout(set=0, binding=0, std140) uniform DefaultUniforms {\n";
				for( const auto& mem : out.members )
				{
					rebuilt << "    " << mem.glslType << " " << mem.name;
					if( mem.arraySize ) rebuilt << "[" << mem.arraySize << "]";
					rebuilt << ";\n";
				}
				rebuilt << "} _ubo;\n";
			}
			for( size_t i = 0; i < out.samplers.size(); i++ )
			{
				// opaque type recovered from the original source line
				std::regex declRe( "uniform\\s+([A-Za-z_]\\w*)\\s+" + out.samplers[i] + "\\s*;" );
				std::smatch dm;
				string stype = "sampler2D";
				if( std::regex_search( source, dm, declRe ) ) stype = dm[1];
				rebuilt << "layout(set=0, binding=" << ( 1 + i ) << ") uniform "
						<< stype << " " << out.samplers[i] << ";\n";
			}
			versionWritten = true;
			continue;
		}
		rebuilt << line << '\n';
	}
	if(! versionWritten )
	{
		// no #version line — prepend everything
		string prefix = "#version 450\n";
		out.source = prefix + rebuilt.str();
	} else
	{
		out.source = rebuilt.str();
	}
	return out;
}

// ---------------------------------------------------------------------
// compile (glslang → SPIR-V)
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
