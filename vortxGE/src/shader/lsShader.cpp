#include <iostream>
#include <vector>
#include <algorithm>

// core
#include "../vortx_debug.h"

// filesystem
#include "../filesystem/lsDir.h"
#include "../filesystem/lsFileIO.h"

// math
#include "../math/lsMath.h"
#include "../math/lsMatrix4x4.h"
#include "../math/lsMatrix3x3.h"

// scene
#include "../scene/lsObject.h"
#include "../scene/lsScene.h"
#include "../scene/lsCamera.h"
#include "../scene/lsLight.h"
#include "../scene/lsLight_Spot.h"
#include "../scene/lsLight_Direct.h"
#include "../scene/lsLightManager.h"

// datablocks
#include "../datablocks/lsBatch_Quad.h"
#include "../datablocks/lsBatch_Lines.h"
#include "../datablocks/lsBatch_Mesh.h"

#include "lsShader.h"

lsShader::lsShader( lsRender *render )
{
	m_render = render;
	
	name = "Not set";
	kind = ShaderKind::None; // Initialize to None

	shader_v = getRender()->CreateShader( LS_VERTEX_SHADER );
	shader_f = getRender()->CreateShader( LS_FRAGMENT_SHADER );
	shader = getRender()->CreateProgram();
	getRender()->AttachShader( shader, shader_v );
	getRender()->AttachShader( shader, shader_f );

	// Initialize all uniform locations to -1
	imvpMatrix = -1;
	ipMatrix = -1;
	imvMatrix = -1;
	imMatrix = -1;
	inMatrix = -1;
	
	iTex_albedo = -1;
	iTex_roughness_metallic = -1;
	iTex_normal = -1;
	
	for(int i = 0; i < 3; ++i)
	{
		isMatrices[i] = -1;
		iTex_shadowmaps[i] = -1;
		iUse_shadowmap[i] = -1;
		iLightType[i] = -1;
		iLightPos[i] = -1;
		iLightDir[i] = -1;
		iLightCutoff[i] = -1;
		iLightOuterCutoff[i] = -1;
		iLightDiffuse[i] = -1;
		iLightSpecular[i] = -1;
		iLightIntensity[i] = -1;
	}

	iLightCount = -1;
	iDebugView = -1;
	iMatAlbedo = -1;
	iMatRoughness = -1;
	iMatMetallic = -1;
}

lsShader::~lsShader()
{
	getRender()->DetachShader( shader, shader_v );
	getRender()->DetachShader( shader, shader_f );
	getRender()->DeleteShader( shader_v );
	getRender()->DeleteShader( shader_f );
	
	getRender()->DeleteProgram( shader );
}

void lsShader::setKind( ShaderKind val ) // Renamed from setShader_type
{
	kind = val; // Store the new kind

	// Bind shader
	getRender()->UseProgram( shader );

	// by shader kind
	switch( kind )
	{
		case ShaderKind::LightTexDNS:
			// Texture 'S' combined roughness (r), metallic (g) and AO (b).
			iTex_roughness_metallic = getRender()->GetUniformLocation( shader, "tex_roughness_metallic" );
			[[fallthrough]];
		case ShaderKind::LightTexDN:
			// Texture normal
			iTex_normal = getRender()->GetUniformLocation( shader, "tex_normal" );
			[[fallthrough]];
		case ShaderKind::LightTexD:
			// Texture 'D' Albedo
			iTex_albedo = getRender()->GetUniformLocation( shader, "tex_albedo" );
			[[fallthrough]];
		case ShaderKind::LightTexNone:
		case ShaderKind::DebugNormals: // Although debug_normals doesn't use all lights, keep light uniform init common
			// Shadow Map uniforms (arrays)
			for(int i = 0; i < 3; ++i) {
				isMatrices[i] = getRender()->GetUniformLocation( shader, ("sMatrices[" + to_string(i) + "]").c_str() );
				iTex_shadowmaps[i] = getRender()->GetUniformLocation( shader, ("tex_shadowmaps[" + to_string(i) + "]").c_str() );
				iUse_shadowmap[i] = getRender()->GetUniformLocation( shader, ("use_shadowmap[" + to_string(i) + "]").c_str() );
			}
			
			// Lights uniforms (arrays)
			iLightCount = getRender()->GetUniformLocation( shader, "vxLightCount" );
			iDebugView = getRender()->GetUniformLocation( shader, "vxDebugView" );
			for(int i = 0; i < 3; ++i) {
				iLightType[i] = getRender()->GetUniformLocation( shader, ("vxLightType[" + to_string(i) + "]").c_str() );
				iLightPos[i] = getRender()->GetUniformLocation( shader, ("vxLightPos[" + to_string(i) + "]").c_str() );
				iLightDir[i] = getRender()->GetUniformLocation( shader, ("vxLightDir[" + to_string(i) + "]").c_str() );
				iLightCutoff[i] = getRender()->GetUniformLocation( shader, ("vxLightCutoff[" + to_string(i) + "]").c_str() );
				iLightOuterCutoff[i] = getRender()->GetUniformLocation( shader, ("vxLightOuterCutoff[" + to_string(i) + "]").c_str() );
				iLightDiffuse[i] = getRender()->GetUniformLocation( shader, ("vxLightDiffuse[" + to_string(i) + "]").c_str() );
				iLightSpecular[i] = getRender()->GetUniformLocation( shader, ("vxLightSpecular[" + to_string(i) + "]").c_str() );
				iLightIntensity[i] = getRender()->GetUniformLocation( shader, ("vxLightIntensity[" + to_string(i) + "]").c_str() );
			}
			
			// Material
			iMatAlbedo = getRender()->GetUniformLocation( shader, "vxMatAlbedo" );
			iMatRoughness = getRender()->GetUniformLocation( shader, "vxMatRoughness" );
			iMatMetallic = getRender()->GetUniformLocation( shader, "vxMatMetallic" );
			
			// Need ModelViewMatrix and ProjectionMatrix
			imvMatrix = getRender()->GetUniformLocation( shader, "mvMatrix" );
			imMatrix = getRender()->GetUniformLocation( shader, "mMatrix" );
			ipMatrix = getRender()->GetUniformLocation( shader, "pMatrix" );
			// Normal Matrix
			inMatrix = getRender()->GetUniformLocation( shader, "nMatrix" );
			break;

		case ShaderKind::Quad:
			// Texture
			iTex_albedo = getRender()->GetUniformLocation( shader, "tex_albedo" );
			[[fallthrough]];
		case ShaderKind::Debug:
			// Need ModelViewMatrix and ProjectionMatrix
			imvpMatrix = getRender()->GetUniformLocation( shader, "mvpMatrix" );
			break;

		case ShaderKind::ShadowMap:
			imvpMatrix = getRender()->GetUniformLocation( shader, "mvpMatrix" );
			break;

		case ShaderKind::Skybox: // Initialize uniforms for skybox shader
			ipMatrix = getRender()->GetUniformLocation(shader, "pMatrix");
			imvMatrix = getRender()->GetUniformLocation(shader, "mvMatrix");
			iTex_albedo = getRender()->GetUniformLocation(shader, "tex_skybox");
			break;
			
		default:
			break;
	} // switch kind
	
	getRender()->UseProgram(0);
}

// Resolve the effective light list for an object: the per-object list
// filled by LinkLightsInScene, else the owning scene's light manager
// (whose per-light active flags are refreshed every frame). Single
// source of truth for BOTH UseShader's shadow binding and Setup_Lights'
// uniform upload — they must never disagree about which light occupies
// which shader slot.
static const std::vector<lsLight*>& resolveLightList( lsObject* object )
{
	const std::vector<lsLight*>& own = object->getLightManager()->getLights();
	if( ! own.empty() ) return own;
	for( lsNode* p = object->getParent(); p; p = p->getParent() )
		if( auto* scn = dynamic_cast<lsScene*>( p ) )
			return scn->getLightManager()->getLights();
	return own; // empty
}

#if defined(VORTX_DEBUG_TOOLS)
// D1.1: VORTX_ONLY_LIGHT=i renders with only light i; VORTX_LIGHTS_OFF=1
// renders ambient-only. Applied to BOTH the shadow-binding loop here and
// Setup_Lights' uniform upload so the two never disagree.
static std::vector<lsLight*> filterDebugLights( const std::vector<lsLight*>& in )
{
	static const int only = []
	{
		const char* e = getenv( "VORTX_ONLY_LIGHT" );
		return e ? atoi( e ) : -1;
	}();
	static const bool off = getenv( "VORTX_LIGHTS_OFF" ) != nullptr;
	if( off ) return {};
	if( only >= 0 )
	{
		if( only < (int)in.size() && in[only] ) return { in[only] };
		return {};
	}
	return in;
}

// D1.2: VORTX_DEBUG_VIEW=lightcount|shadow|normals|depth -> vxDebugView
static int debugViewMode()
{
	static const int mode = []
	{
		const char* dv = getenv( "VORTX_DEBUG_VIEW" );
		if( ! dv ) return 0;
		const std::string s = dv;
		if( s == "lightcount" ) return 1;
		if( s == "shadow" ) return 2;
		if( s == "normals" ) return 3;
		if( s == "depth" ) return 4;
		return 0;
	}();
	return mode;
}
#endif

void lsShader::UseShader( lsCamera *cam, lsObject *object, lsPass* /* pass */, lsBatch *batch )
{
#ifdef VORTX_DEBUG_SHADERS
	cout << "lsShader::UseShader() - Called" << endl;
#endif

	lsBatch_Quad *batch_quad;
	lsBatch_Mesh *batch_mesh;

	lsMatrix4x4 mvpMatrix;
	lsMatrix4x4 mvMatrix;
	lsMatrix4x4 pMatrix;
	lsMatrix4x4 mMatrix; // Declare mMatrix locally. This was the fix.
	lsMatrix3x3 m_eMatrix;      // extract 3x3
	lsMatrix3x3 m_iMatrix;      // inverse modelview matrix
	lsMatrix3x3 nMatrix;        // normal matrix

	// assign
	batch_mesh = (lsBatch_Mesh*)batch;
	batch_quad = (lsBatch_Quad*)batch;

	getRender()->UseProgram( shader );
	
	switch( kind ) // Changed from shader_type to kind
	{
		case ShaderKind::LightTexDNS:
			// Texture 'S' combined roughness (r), metallic (g) and AO (b).
			getRender()->ActiveTexture( LS_TEXTURE2 );
			getRender()->BindTexture( LS_TEXTURE_2D, batch_mesh->tex_roughness_metallic );
			getRender()->Uniform1i( iTex_roughness_metallic, 2 );
			[[fallthrough]];
		case ShaderKind::LightTexDN:
			// Texture normal
			getRender()->ActiveTexture( LS_TEXTURE1 );
			getRender()->BindTexture( LS_TEXTURE_2D, batch_mesh->tex_normal );
			getRender()->Uniform1i( iTex_normal, 1 );
			[[fallthrough]];
		case ShaderKind::LightTexD:
			// Texture 'D' Albedo
			getRender()->ActiveTexture( LS_TEXTURE0 );
			getRender()->BindTexture( LS_TEXTURE_2D, batch_mesh->tex_albedo );
			getRender()->Uniform1i( iTex_albedo, 0 );
			[[fallthrough]];
		case ShaderKind::LightTexNone:
			// Shadow Map & Light Uniforms
			{
#if defined(VORTX_DEBUG_TOOLS)
				const std::vector<lsLight*> lights = filterDebugLights(
					resolveLightList( object ) );
#else
				const auto& lights = resolveLightList( object );
#endif
				// FIX 2026-08-11 (Vulkan dark floor, 2/4 — see Plan.md "D1").
				// Slot compaction MUST match Setup_Lights exactly (active
				// filter + running index) — shader slot j samples
				// tex_shadowmaps[j]/sMatrices[j] for the light whose
				// properties Set_Light(j, ...) uploaded.
				int slot = 0;
				for (int i = 0; i < (int)lights.size() && slot < 3; ++i)
				{
					lsLight* light = lights[i];
					if (light && light->getActive())
					{
						// Bind shadow map texture for this light
#if defined(VORTX_DEBUG_TOOLS)
						// DIAGNOSTIC: VORTX_DISABLE_SHADOWS=1 forces
						// use_shadowmap=0 for shadow-casting lights too
						static const bool s_disableShadows = !!getenv( "VORTX_DISABLE_SHADOWS" );
#else
						constexpr bool s_disableShadows = false;
#endif
						if (light->getHas_shadowmap() && !s_disableShadows)
						{
							getRender()->ActiveTexture(LS_TEXTURE3 + slot); // texture units 3, 4, 5 for shadow maps
							getRender()->BindTexture(LS_TEXTURE_2D, light->getTex_shadowmap());
							getRender()->Uniform1i(iTex_shadowmaps[slot], 3 + slot);
							getRender()->Uniform1i(iUse_shadowmap[slot], 1);
							getRender()->UniformMatrix4fv(isMatrices[slot], 1, LS_FALSE, light->ShadowmapMatrix.matrix);
						} else {
							// DIAGNOSTIC: VORTX_DISABLE_SHADOWS=1 forces use_shadowmap=0
							// for shadow-casting lights too, to verify whether the
							// 'ambient-only' symptom is shadow-related.
							getRender()->Uniform1i(iUse_shadowmap[slot], 0);
						}
						slot++;
					}
				}
				// Pass the count of active lights to the shader
				getRender()->Uniform1i(iLightCount, slot);
#if defined(VORTX_DEBUG_TOOLS)
				getRender()->Uniform1i( iDebugView, debugViewMode() );
#else
				getRender()->Uniform1i( iDebugView, 0 );
#endif
				Setup_Lights(cam, object); // Now Setup_Lights will pass all light properties
			}
			
			// Material
			if( batch->getBatch_type() == LSBT_MESH )
			{
				getRender()->Uniform4f( iMatAlbedo, batch_mesh->m_albedo.x, batch_mesh->m_albedo.y, batch_mesh->m_albedo.z, batch_mesh->m_albedo.w );
				getRender()->Uniform1f( iMatRoughness, batch_mesh->m_roughness );
				getRender()->Uniform1f( iMatMetallic, batch_mesh->m_metallic );
			}
			
			// Matrix data
			pMatrix = cam->frustum.getProjectionMatrix();
			mvMatrix = cam->ViewMatrix * object->ModelMatrix;
			mMatrix = object->ModelMatrix;
			// Normal Matrix
			m_eMatrix.ExtractRotationMatrix4x4( mvMatrix );
			m_iMatrix.Inverse( m_eMatrix );
			nMatrix.Transpose( m_iMatrix );
			
			getRender()->UniformMatrix4fv( ipMatrix, 1, LS_FALSE, pMatrix.matrix );
			getRender()->UniformMatrix4fv( imvMatrix, 1, LS_FALSE, mvMatrix.matrix );
			getRender()->UniformMatrix4fv( imMatrix, 1, LS_FALSE, mMatrix.matrix );
			getRender()->UniformMatrix3fv( inMatrix, 1, LS_FALSE, nMatrix.matrix );
			break;

		case ShaderKind::DebugNormals:
			// Material
			if( batch->getBatch_type() == LSBT_MESH )
			{
				getRender()->Uniform4f( iMatAlbedo, batch_mesh->m_albedo.x, batch_mesh->m_albedo.y, batch_mesh->m_albedo.z, batch_mesh->m_albedo.w );
				getRender()->Uniform1f( iMatRoughness, batch_mesh->m_roughness );
				getRender()->Uniform1f( iMatMetallic, batch_mesh->m_metallic );
			}
			
			// Matrix data
			pMatrix = cam->frustum.getProjectionMatrix();
			mvMatrix = cam->ViewMatrix * object->ModelMatrix;
			// Normal Matrix
			m_eMatrix.ExtractRotationMatrix4x4( mvMatrix );
			m_iMatrix.Inverse( m_eMatrix );
			nMatrix.Transpose( m_iMatrix );
			
			getRender()->UniformMatrix4fv( ipMatrix, 1, LS_FALSE, pMatrix.matrix );
			getRender()->UniformMatrix4fv( imvMatrix, 1, LS_FALSE, mvMatrix.matrix );
			getRender()->UniformMatrix3fv( inMatrix, 1, LS_FALSE, nMatrix.matrix );
			break;

		case ShaderKind::Quad:
			pMatrix = cam->frustum.getProjectionMatrix();
			mvMatrix = cam->ViewMatrix * object->ModelMatrix;
			mvpMatrix = pMatrix * mvMatrix;
			getRender()->UniformMatrix4fv( imvpMatrix, 1, LS_FALSE, mvpMatrix.matrix );
			
			// Textures
			getRender()->ActiveTexture( LS_TEXTURE0 );
			getRender()->BindTexture( LS_TEXTURE_2D, batch_quad->getTex_albedo() );
			getRender()->Uniform1i( iTex_albedo, 0 );
			break;
		
		case ShaderKind::Debug:
			pMatrix = cam->frustum.getProjectionMatrix();
			mvMatrix = cam->ViewMatrix * object->ModelMatrix;
			mvpMatrix = pMatrix * mvMatrix;
			getRender()->UniformMatrix4fv( imvpMatrix, 1, LS_FALSE, mvpMatrix.matrix );
			break;

		case ShaderKind::ShadowMap:
			pMatrix = cam->frustum.getProjectionMatrix();
			mvMatrix = cam->ViewMatrix * object->ModelMatrix;
			mvpMatrix = pMatrix * mvMatrix;
			getRender()->UniformMatrix4fv(imvpMatrix, 1, LS_FALSE, mvpMatrix.matrix);
			break;

		case ShaderKind::Skybox:
			pMatrix = cam->frustum.getProjectionMatrix();
			// For skybox, remove translation from camera's view matrix
			// This keeps the skybox centered on the camera, giving infinite distance
			mvMatrix = cam->ViewMatrix.extractRotation(); 
			
			getRender()->UniformMatrix4fv(ipMatrix, 1, LS_FALSE, pMatrix.matrix);
			getRender()->UniformMatrix4fv(imvMatrix, 1, LS_FALSE, mvMatrix.matrix);

			// Bind skybox texture
			getRender()->ActiveTexture(LS_TEXTURE0);
			// Assuming tex_albedo holds the skybox texture, cast batch to lsBatch_Mesh
			if (batch->getBatch_type() == LSBT_MESH) {
				lsBatch_Mesh* mesh_batch = static_cast<lsBatch_Mesh*>(batch);
				getRender()->BindTexture(LS_TEXTURE_2D, mesh_batch->tex_albedo); 
			}
			getRender()->Uniform1i(iTex_albedo, 0);
			break;

		default:
#ifdef VORTX_DEBUG_SHADERS
			cout << "ERROR: lsShader::UseShader() - Unsupported shader type" << endl;
#endif
			break;
	}
}

/**
 * @brief Load and compile shader from file
 * 
 * Loads vertex and fragment shaders from the data/shaders directory.
 * The shader files are named as:
 * - name.vert (vertex shader)
 * - name.frag (fragment shader)
 * 
 * The function:
 * 1. Reads .vert and .frag files from disk
 * 2. Compiles each shader with OpenGL
 * 3. Checks for compilation errors
 * 4. Links them into a shader program
 * 5. Determines shader kind based on filename
 * 
 * Shader loading process:
 * 1. getRender()->CreateShader() - Create shader objects
 * 2. getRender()->ShaderSource() - Load GLSL source code
 * 3. getRender()->CompileShader() - Compile to GPU bytecode
 * 4. getRender()->CreateProgram() - Create program object
 * 5. getRender()->AttachShader() - Attach shaders to program
 * 6. getRender()->LinkProgram() - Link into executable
 * 
 * @param name Shader name (without extension)
 * 
 * @note Shader files expected in: data/shaders/{name}.vert and .frag
 * @note Use setKind() to configure uniforms after loading
 */

void lsShader::Load_Shader( const string& name )
{
	this->name = name;
	kind = ShaderKind::None; // Initialize to None

	// Set shader kind based on name (will be replaced by manager's map)
	if( name == "" )
	{
		cout << "ERROR: lsShader::Load_Shader() - Shader must have a name" << endl;
		return;
	}

	// Load and compile the shader
	string str;
	string basepath;
	lsDir dir;

	// built path
	basepath = dir.getexepath();
	basepath += "data/shaders/";

	// Vertex shader
	#ifdef VORTX_DEBUG_SHADERS
	cout << "Load Shader : " << basepath + name + ".vert" << endl;
	#endif
	
	lsFileIO vshaderfile;
	if(! vshaderfile.open( basepath + name + ".vert", LSIO_LINE_READ ) )
	{
		cout << "ERROR: lsShader::Load_Shader() - Vertex Shader not found" << endl;
		return;
	}

	// Make sure we have end of line. Easier to read on debug.
	str = "";
	while( vshaderfile.readline() )
	{
		str += vshaderfile.getLine();
		str += "\n";
	}

	const lsChar* source = (lsChar*)str.c_str();
	getRender()->ShaderSource( shader_v, 1, &source, NULL );

	// Compile it
	#ifdef VORTX_DEBUG_SHADERS
	cout << "Compile Vertex Shader" << endl;
	#endif
	
	getRender()->CompileShader( shader_v );
	debug( shader_v );

	// Fragment shader
	// file
	#ifdef VORTX_DEBUG_SHADERS
	cout << "Load Shader : " << basepath + name + ".frag" << endl;
	#endif
	
	lsFileIO fshaderfile;

	if(! fshaderfile.open( basepath + name + ".frag", LSIO_LINE_READ ) )
	{
		cout << "ERROR: lsShader::Load_Shader() - Fragment Shader not found" << endl;
		return;
	}

	// Make sure we have end of line.
	str = "";
	while( fshaderfile.readline() )
	{
		str += fshaderfile.getLine();
		str += "\n";
	}

	source = (lsChar*)str.c_str();
	getRender()->ShaderSource( shader_f, 1, &source, NULL );

	// Compile it
	#ifdef VORTX_DEBUG_SHADERS
	cout << "Compile Fragment Shader" << endl;
	#endif
	
	getRender()->CompileShader( shader_f );
	debug( shader_f );

	// Link Program
	#ifdef VORTX_DEBUG_SHADERS
	cout << "Linking shader" << endl;
	#endif
	
	getRender()->LinkProgram( shader );

	// Set shader kind based on name (will be replaced by manager's map)
	if(! debugProgram() )
	{
		cout << "ERROR: lsShader::Load_Shader() - Program link issue" << endl;
	} else
	{
		// All went well
		#ifdef VORTX_DEBUG_SHADERS
		cout << "Shader program linked successfully" << endl;
		#endif
	}
	
	// Set kind based on name (will be replaced by manager's map)
	if( name == "debug" )
	{
		setKind( ShaderKind::Debug );
	} else  if( name == "quad" )
	{
		setKind( ShaderKind::Quad );
	} else  if( name == "light_tex_d" )
	{
		setKind( ShaderKind::LightTexD );
	} else  if( name == "light_tex_dn" )
	{
		setKind( ShaderKind::LightTexDN );
	} else  if( name == "light_tex_dns" )
	{
		setKind( ShaderKind::LightTexDNS );
	} else  if( name == "light_tex_none" )
	{
		setKind( ShaderKind::LightTexNone );
	} else  if( name == "debug_normals" )
	{
		setKind( ShaderKind::DebugNormals );
	} else  if( name == "shadowmap" )
	{
		setKind( ShaderKind::ShadowMap );
	} else  if( name == "skybox" ) // Add this
	{
		setKind( ShaderKind::Skybox );
	}
}

/**
 * @brief Check shader compilation/link status
 * 
 * Queries the OpenGL info log for shader compilation or linking errors.
 * If any errors are found, they are printed to console.
 * 
 * This should be called after:
 * - getRender()->CompileShader() to check for compilation errors
 * - getRender()->LinkProgram() to check for linking errors
 * 
 * @param s Shader or program object to check
 */

void lsShader::debug( lsShaderID s )
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	getRender()->GetShaderiv( s, LS_INFO_LOG_LENGTH, &infologLength );
	if( infologLength > 1 )
	{
		infoLog = new char[ infologLength+1 ];
		getRender()->GetShaderInfoLog( s, infologLength, &charsWritten, infoLog );
		cout << "InfoLog Shader....." << endl;
		cout << infoLog << endl;
		delete [] infoLog;
	}
}

bool lsShader::debugProgram()
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	getRender()->GetProgramiv( shader, LS_INFO_LOG_LENGTH, &infologLength );
	if( infologLength > 1 )
	{
		infoLog = new char[ infologLength+1 ];
		getRender()->GetProgramInfoLog( shader, infologLength, &charsWritten, infoLog );
		cout << "InfoLog Program....." << endl;
		cout << infoLog << endl;
		delete [] infoLog;

		return false;
	}
	return true;
}

void lsShader::Setup_Lights( lsCamera *camera, lsObject *object )
{
	// Per-object light lists are filled once by LinkLightsInScene at
	// scene-load time. Objects created later (lsObjectEmitter pool,
	// runtime game code) are never linked and would render unlit, so
	// fall back to the owning scene's light manager — its per-light
	// active flags are refreshed every frame by setActiveLights().
	const vector<lsLight*>* lights = &resolveLightList( object );

#if defined(VORTX_DEBUG_TOOLS)
	const vector<lsLight*> filtered = filterDebugLights( *lights );
	lights = &filtered;
#endif
	int num_lights = min((int)lights->size(), 3); // Shader supports max 3 lights
	int i = 0;

	for( auto light : *lights )
	{
		// Make sure we break when i it at the light count.
		if(i > num_lights - 1) break;
		
		if (light && light->getActive())
		{
			Set_Light(i, camera, light, object);
			i++;
		}
	}
	// Set light count i should be the light count.
	getRender()->Uniform1i(iLightCount, i);
}

void lsShader::Set_Light( int i, lsCamera *camera, lsLight *light, __attribute__((unused)) lsObject *object )
{
	if (i < 0 || i >= 3) return; // Only support up to 3 lights for now

	lsMatrix4x4 light_in_view_space_matrix;
	
	// Light position in camera Space
	light_in_view_space_matrix = camera->ViewMatrix * light->ModelMatrix;
	
	lsVector3 lightDir_view_space;
	if(light->getLight_type() == LSLT_SPOT)
	{
		// The light's direction is its local -Z axis.
		lsVector3 lightDir_local(0.0f, 0.0f, -1.0f);

		// Transform it to world space using the light's model matrix (rotation only)
		lsVector3 lightDir_world = light->ModelMatrix.Rotate(lightDir_local);

		// Transform it to view space using the camera's view matrix (rotation only)
		lightDir_view_space = camera->ViewMatrix.Rotate(lightDir_world);
		lightDir_view_space.normalize();
	} else if(light->getLight_type() == LSLT_DIRECT)
	{
		lsLight_Direct* l = static_cast<lsLight_Direct*>(light);
		lightDir_view_space = camera->ViewMatrix.Rotate(l->m_lightDirection); // Use camera's rotation for direct light direction in view space
		lightDir_view_space.normalize();
	}
	
	if (iLightPos[i] != -1) getRender()->Uniform3f( iLightPos[i], light_in_view_space_matrix.matrix[12], light_in_view_space_matrix.matrix[13], light_in_view_space_matrix.matrix[14] );
	if (iLightType[i] != -1) getRender()->Uniform1i( iLightType[i], light->getLight_type() );
	if (iLightDiffuse[i] != -1) getRender()->Uniform4f( iLightDiffuse[i], light->diffuse.x, light->diffuse.y, light->diffuse.z, light->diffuse.w );
	if (iLightSpecular[i] != -1) getRender()->Uniform4f( iLightSpecular[i], light->specular.x, light->specular.y, light->specular.z, light->specular.w );
	if (iLightIntensity[i] != -1) getRender()->Uniform1f( iLightIntensity[i], light->getIntensity() );

	// Only set direction and cutoff for spot/direct lights
	if (light->getLight_type() == LSLT_SPOT || light->getLight_type() == LSLT_DIRECT)
	{
		if (iLightDir[i] != -1) getRender()->Uniform3f( iLightDir[i], lightDir_view_space.x, lightDir_view_space.y, lightDir_view_space.z );
		if (iLightCutoff[i] != -1) getRender()->Uniform1f( iLightCutoff[i], cos(light->spot_cutoff * ANG2RAD) );
		if (iLightOuterCutoff[i] != -1) getRender()->Uniform1f( iLightOuterCutoff[i], cos(light->spot_outer_cutoff * ANG2RAD) );
	}
}
