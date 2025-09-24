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
#include "../scene/lsCamera.h"
#include "../scene/lsLight.h"
#include "../scene/lsLightManager.h"

// datablocks
#include "../datablocks/lsBatch_Quad.h"
#include "../datablocks/lsBatch_Lines.h"
#include "../datablocks/lsBatch_Mesh.h"

#include "lsShader.h"


lsShader::lsShader( lsOpenGL *opengl )
{
	ogl = opengl;

	next_shader = NULL;
	prev_shader = NULL;

	name = "Not set";
	shader_type = LSST_NONE;

	shader_v = glCreateShader( GL_VERTEX_SHADER );
	shader_f = glCreateShader( GL_FRAGMENT_SHADER );
	shader = glCreateProgram();
	glAttachShader( shader, shader_v );
	glAttachShader( shader, shader_f );

}

lsShader::~lsShader()
{
	glDetachShader( shader, shader_v );
	glDetachShader( shader, shader_f );
	glDeleteShader( shader_v );
	glDeleteShader( shader_f );

	glDeleteProgram( shader );
}

void lsShader::setShader_type( int val )
{
	shader_type = val;

	// Bind shader
	glUseProgram( shader );

	// by shader type
	switch( shader_type )
	{
	case LSST_LIGHT_TEX_DNS:
		// Texture
		iTex_specular = glGetUniformLocation( shader, "tex_specular" );
		[[fallthrough]];
	case LSST_LIGHT_TEX_DN:
		// Texture
		iTex_normal = glGetUniformLocation( shader, "tex_normal" );
		[[fallthrough]];
	case LSST_LIGHT_TEX_D:
		// Texture
		iTex_diffuse = glGetUniformLocation( shader, "tex_diffuse" );
		[[fallthrough]];
	case LSST_LIGHT_TEX_NONE:
	case LSST_DEBUG_NORMALS:
		// Shadow Matrix
		isMatrix = glGetUniformLocation( shader, "sMatrix" );
		iTex_shadowmap = glGetUniformLocation( shader, "tex_shadowmap" );
		iUse_shadowmap = glGetUniformLocation( shader, "use_shadowmap" );
		// Lights
		iLightCount = glGetUniformLocation( shader, "vxLightCount" );
		// Light 1
		iLightType1 = glGetUniformLocation( shader, "vxLightType[0]" );
		iLightPos1 = glGetUniformLocation( shader, "vxLightPos[0]" );
		iLightDir1 = glGetUniformLocation( shader, "vxLightDir[0]" );
		iLightCutoff1 = glGetUniformLocation( shader, "vxLightCutoff[0]" );
		iLightOuterCutoff1 = glGetUniformLocation( shader, "vxLightOuterCutoff[0]" );
		iLightDiffuse1 = glGetUniformLocation( shader, "vxLightDiffuse[0]" );
		iLightSpecular1 = glGetUniformLocation( shader, "vxLightSpecular[0]" );
		iLightIntensity1 = glGetUniformLocation( shader, "vxLightIntensity[0]" );
		// Light 2
		iLightType2 = glGetUniformLocation( shader, "vxLightType[1]" );
		iLightPos2 = glGetUniformLocation( shader, "vxLightPos[1]" );
		iLightDir2 = glGetUniformLocation( shader, "vxLightDir[1]" );
		iLightCutoff2 = glGetUniformLocation( shader, "vxLightCutoff[1]" );
		iLightOuterCutoff2 = glGetUniformLocation( shader, "vxLightOuterCutoff[1]" );
		iLightDiffuse2 = glGetUniformLocation( shader, "vxLightDiffuse[1]" );
		iLightSpecular2 = glGetUniformLocation( shader, "vxLightSpecular[1]" );
		iLightIntensity2 = glGetUniformLocation( shader, "vxLightIntensity[1]" );
		// Light 3
		iLightType3 = glGetUniformLocation( shader, "vxLightType[2]" );
		iLightPos3 = glGetUniformLocation( shader, "vxLightPos[2]" );
		iLightDir3 = glGetUniformLocation( shader, "vxLightDir[2]" );
		iLightCutoff3 = glGetUniformLocation( shader, "vxLightCutoff[2]" );
		iLightOuterCutoff3 = glGetUniformLocation( shader, "vxLightOuterCutoff[2]" );
		iLightDiffuse3 = glGetUniformLocation( shader, "vxLightDiffuse[2]" );
		iLightSpecular3 = glGetUniformLocation( shader, "vxLightSpecular[2]" );
		iLightIntensity3 = glGetUniformLocation( shader, "vxLightIntensity[2]" );
		
		// Material
		iMatAmbiant = glGetUniformLocation( shader, "vxMatAmbiant" );
		iMatDiffuse = glGetUniformLocation( shader, "vxMatDiffuse" );
		iMatSpecular = glGetUniformLocation( shader, "vxMatSpecular" );
		iMatShininess = glGetUniformLocation( shader, "vxMatShininess" );
		iMatGlow = glGetUniformLocation( shader, "vxMatGlow" );

		// Need ModelViewMatrix and ProjectionMatrix
		imvMatrix = glGetUniformLocation( shader, "mvMatrix" );
		imMatrix = glGetUniformLocation( shader, "mMatrix" );
		ipMatrix = glGetUniformLocation( shader, "pMatrix" );
		// Normal Matrix
		inMatrix = glGetUniformLocation( shader, "nMatrix" );
		break;

	case LSST_QUAD:
		// Texture
		iTex_diffuse = glGetUniformLocation( shader, "tex_diffuse" );
		[[fallthrough]];
	case LSST_DEBUG:
		// Need ModelViewMatrix and ProjectionMatrix
		imvpMatrix = glGetUniformLocation( shader, "mvpMatrix" );
		break;

	case LSST_SHADOWMAP:
		imvpMatrix = glGetUniformLocation( shader, "mvpMatrix" );
		break;

	} // switch shader_type
	
	glUseProgram(0);
}

void lsShader::UseShader( lsCamera *cam, lsObject *object, __attribute__((unused)) lsPass *pass, lsBatch *batch )
{
#ifdef VORTX_DEBUG_SHADERS
	cout << "lsShader::UseShader() - Called" << endl;
#endif

	lsBatch_Quad *batch_quad;
	lsBatch_Mesh *batch_mesh;

	lsMatrix4x4 mvpMatrix;
	lsMatrix4x4 mvMatrix;
	lsMatrix4x4 mMatrix;
	lsMatrix4x4 pMatrix;
	lsMatrix3x3 m_eMatrix;      // extract 3x3
	lsMatrix3x3 m_iMatrix;      // inverse modelview matrix
	lsMatrix3x3 nMatrix;        // normal matrix

	// assign
	batch_mesh = (lsBatch_Mesh*)batch;
	batch_quad = (lsBatch_Quad*)batch;

	glUseProgram( shader );
	
	switch( shader_type )
	{
		case LSST_LIGHT_TEX_DNS:
			// Texture specular
			glActiveTexture( GL_TEXTURE2 );
			glBindTexture( GL_TEXTURE_2D, batch_mesh->tex_specular );
			glUniform1i( iTex_specular, 2 );
			[[fallthrough]];
		case LSST_LIGHT_TEX_DN:
			// Texture normal
			glActiveTexture( GL_TEXTURE1 );
			glBindTexture( GL_TEXTURE_2D, batch_mesh->tex_normal );
			glUniform1i( iTex_normal, 1 );
			[[fallthrough]];
		case LSST_LIGHT_TEX_D:
			// Texture diffuse
			glActiveTexture( GL_TEXTURE0 );
			glBindTexture( GL_TEXTURE_2D, batch_mesh->tex_diffuse );
			glUniform1i( iTex_diffuse, 0 );
			[[fallthrough]];
		case LSST_LIGHT_TEX_NONE:
			// Shadow Map
			{
				// TODO: Support multiple shadows, move to light uniform
				lsLight* shadow_light = nullptr;
				if (object->getReceives_shadows())
				{
					const auto& lights = object->getLightManager()->getLights();
					for (lsLight* light : lights)
					{
						if (light && light->getHas_shadowmap())
						{
							shadow_light = light;
							break; // Use the first shadow-casting light found
						}
					}
				}

				if (shadow_light)
				{
					glUniform1i(iUse_shadowmap, 1);
					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_2D, shadow_light->getTex_shadowmap());
					glUniform1i(iTex_shadowmap, 3);
					glUniformMatrix4fv(isMatrix, 1, GL_FALSE, shadow_light->ShadowmapMatrix.matrix);
				} else
				{
					glUniform1i(iUse_shadowmap, 0);
				}
			}
			
			// Material
			if( batch->getBatch_type() == LSBT_MESH )
			{
				glUniform4f( iMatAmbiant, batch_mesh->ambiant.x, batch_mesh->ambiant.y, batch_mesh->ambiant.z, batch_mesh->ambiant.w );
				glUniform4f( iMatDiffuse, batch_mesh->diffuse.x, batch_mesh->diffuse.y, batch_mesh->diffuse.z, batch_mesh->diffuse.w );
				glUniform4f( iMatSpecular, batch_mesh->specular.x, batch_mesh->specular.y, batch_mesh->specular.z, batch_mesh->specular.w );
				glUniform1f( iMatShininess, batch_mesh->shininess );
				glUniform1f( iMatGlow, batch_mesh->glow );
			}
			
			// Matrix data
			pMatrix = cam->frustum.getProjectionMatrix();
			mvMatrix = cam->ViewMatrix * object->ModelMatrix;
			mMatrix = object->ModelMatrix;
			// Normal Matrix
			m_eMatrix.ExtractRotationMatrix4x4( mvMatrix );
			m_iMatrix.Inverse( m_eMatrix );
			nMatrix.Transpose( m_iMatrix );
			
			glUniformMatrix4fv( ipMatrix, 1, GL_FALSE, pMatrix.matrix );
			glUniformMatrix4fv( imvMatrix, 1, GL_FALSE, mvMatrix.matrix );
			glUniformMatrix4fv( imMatrix, 1, GL_FALSE, mMatrix.matrix );
			glUniformMatrix3fv( inMatrix, 1, GL_FALSE, nMatrix.matrix );
			break;

		case LSST_DEBUG_NORMALS:
			// Material
			if( batch->getBatch_type() == LSBT_MESH )
			{
				glUniform4f( iMatAmbiant, batch_mesh->ambiant.x, batch_mesh->ambiant.y, batch_mesh->ambiant.z, batch_mesh->ambiant.w );
				glUniform4f( iMatDiffuse, batch_mesh->diffuse.x, batch_mesh->diffuse.y, batch_mesh->diffuse.z, batch_mesh->diffuse.w );
				glUniform4f( iMatSpecular, batch_mesh->specular.x, batch_mesh->specular.y, batch_mesh->specular.z, batch_mesh->specular.w );
				glUniform1f( iMatShininess, batch_mesh->shininess );
				glUniform1f( iMatGlow, batch_mesh->glow );
			}
			
			// Matrix data
			pMatrix = cam->frustum.getProjectionMatrix();
			mvMatrix = cam->ViewMatrix * object->ModelMatrix;
			// Normal Matrix
			m_eMatrix.ExtractRotationMatrix4x4( mvMatrix );
			m_iMatrix.Inverse( m_eMatrix );
			nMatrix.Transpose( m_iMatrix );
			
			glUniformMatrix4fv( ipMatrix, 1, GL_FALSE, pMatrix.matrix );
			glUniformMatrix4fv( imvMatrix, 1, GL_FALSE, mvMatrix.matrix );
			glUniformMatrix3fv( inMatrix, 1, GL_FALSE, nMatrix.matrix );
			break;

		case LSST_QUAD:
			pMatrix = cam->frustum.getProjectionMatrix();
			mvMatrix = cam->ViewMatrix * object->ModelMatrix;
			mvpMatrix = pMatrix * mvMatrix;
			glUniformMatrix4fv( imvpMatrix, 1, GL_FALSE, mvpMatrix.matrix );
			
			// Textures
			glActiveTexture( GL_TEXTURE0 );
			glBindTexture( GL_TEXTURE_2D, batch_quad->getTex_diffuse() );
			glUniform1i( iTex_diffuse, 0 );
			break;
		
		case LSST_DEBUG:
			pMatrix = cam->frustum.getProjectionMatrix();
			mvMatrix = cam->ViewMatrix * object->ModelMatrix;
			mvpMatrix = pMatrix * mvMatrix;
			glUniformMatrix4fv( imvpMatrix, 1, GL_FALSE, mvpMatrix.matrix );
			break;

		case LSST_SHADOWMAP:
			pMatrix = cam->frustum.getProjectionMatrix();
			mvMatrix = cam->ViewMatrix * object->ModelMatrix;
			mvpMatrix = pMatrix * mvMatrix;
			glUniformMatrix4fv(imvpMatrix, 1, GL_FALSE, mvpMatrix.matrix);
			break;
		
		default:
#ifdef VORTX_DEBUG_SHADERS
			cout << "ERROR: lsShader::UseShader() - Unsupported shader type" << endl;
#endif
			break;
	}
}

void lsShader::Load_Shader( const string& name )
{
	this->name = name;
	shader_type = LSST_NONE;

	// Set shader type
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
		shader_type = LSST_NONE;
		return;
	}

	// Make sure we have end of line. Easier to read on debug.
	str = "";
	while( vshaderfile.readline() )
	{
		str += vshaderfile.getLine();
		str += "\n";
	}

	const GLchar* source = (GLchar*)str.c_str();
	glShaderSource( shader_v, 1, &source, NULL );

	// Compile it
#ifdef VORTX_DEBUG_SHADERS
	cout << "Compile Vertex Shader" << endl;
#endif
	glCompileShader( shader_v );
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
		shader_type = LSST_NONE;
		return;
	}

	// Make sure we have end of line.
	str = "";
	while( fshaderfile.readline() )
	{
		str += fshaderfile.getLine();
		str += "\n";
	}

	source = (GLchar*)str.c_str();
	glShaderSource( shader_f, 1, &source, NULL );

	// Compile it
#ifdef VORTX_DEBUG_SHADERS
	cout << "Compile Fragment Shader" << endl;
#endif
	glCompileShader( shader_f );
	debug( shader_f );

	// Link Program
#ifdef VORTX_DEBUG_SHADERS
	cout << "Linking shader" << endl;
#endif
	glLinkProgram( shader );

	// Set shader type
	if(! debugProgram() )
	{
		cout << "ERROR: lsShader::Load_Shader() - Program link issue" << endl;
	} else
	{
		// All went well
#ifdef VORTX_DEBUG_SHADERS
		cout << "Shader pprogram linked successfully" << endl;
#endif
	} // if(! debugProgram() )

	// Always set out type even if we get link issues.
	if( name == "debug" )
	{
		setShader_type( LSST_DEBUG );
	} else  if( name == "quad" )
	{
		setShader_type( LSST_QUAD );
	} else  if( name == "light_tex_d" )
	{
		setShader_type( LSST_LIGHT_TEX_D );
	} else  if( name == "light_tex_dn" )
	{
		setShader_type( LSST_LIGHT_TEX_DN );
	} else  if( name == "light_tex_dns" )
	{
		setShader_type( LSST_LIGHT_TEX_DNS );
	} else  if( name == "light_tex_none" )
	{
		setShader_type( LSST_LIGHT_TEX_NONE );
	} else  if( name == "debug_normals" )
	{
		setShader_type( LSST_DEBUG_NORMALS );
	} else  if( name == "shadowmap" )
	{
		setShader_type( LSST_SHADOWMAP );
	}
}

void lsShader::debug( GLuint s )
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	glGetShaderiv( s, GL_INFO_LOG_LENGTH, &infologLength );
	if( infologLength > 1 )
	{
		infoLog = new char[ infologLength+1 ];
		glGetShaderInfoLog( s, infologLength, &charsWritten, infoLog );
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

	glGetProgramiv( shader, GL_INFO_LOG_LENGTH, &infologLength );
	if( infologLength > 1 )
	{
		infoLog = new char[ infologLength+1 ];
		glGetProgramInfoLog( shader, infologLength, &charsWritten, infoLog );
		cout << "InfoLog Program....." << endl;
		cout << infoLog << endl;
		delete [] infoLog;

		return false;
	}
	return true;
}

void lsShader::Setup_Lights( lsCamera *camera, lsObject *object )
{
	const auto& lights = object->getLightManager()->getLights();
	int light_count = std::min((int)lights.size(), 3); // Shader supports max 3 lights

	for (int i = 0; i < light_count; ++i)
	{
		lsLight* light = lights[i];
		if (light)
		{
			Set_Light(i, camera, light, object);
		}
	}

	// Set count
	glUniform1i(iLightCount, light_count);
}

void lsShader::Set_Light( int i, lsCamera *camera, lsLight *light, __attribute__((unused)) lsObject *object )
{
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
	}
	
	switch( i )
	{
		case 0:
			glUniform3f( iLightPos1, light_in_view_space_matrix.matrix[12], light_in_view_space_matrix.matrix[13], light_in_view_space_matrix.matrix[14] );
			glUniform1i( iLightType1, light->getLight_type() );
			glUniform4f( iLightDiffuse1, light->diffuse.x, light->diffuse.y, light->diffuse.z, light->diffuse.w );
			glUniform4f( iLightSpecular1, light->specular.x, light->specular.y, light->specular.z, light->specular.w );
			glUniform1f( iLightIntensity1, light->getIntensity() );
			if (light->getLight_type() == LSLT_SPOT)
			{
				glUniform3f( iLightDir1, lightDir_view_space.x, lightDir_view_space.y, lightDir_view_space.z );
				glUniform1f( iLightCutoff1, cos(light->spot_cutoff * ANG2RAD) );
				glUniform1f( iLightOuterCutoff1, cos(light->spot_outer_cutoff * ANG2RAD) );
			}
			break;
		
		case 1:
			glUniform3f( iLightPos2, light_in_view_space_matrix.matrix[12], light_in_view_space_matrix.matrix[13], light_in_view_space_matrix.matrix[14] );
			glUniform1i( iLightType2, light->getLight_type() );
			glUniform4f( iLightDiffuse2, light->diffuse.x, light->diffuse.y, light->diffuse.z, light->diffuse.w );
			glUniform4f( iLightSpecular2, light->specular.x, light->specular.y, light->specular.z, light->specular.w );
			glUniform1f( iLightIntensity2, light->getIntensity() );
			if (light->getLight_type() == LSLT_SPOT) {
				glUniform3f( iLightDir2, lightDir_view_space.x, lightDir_view_space.y, lightDir_view_space.z );
				glUniform1f( iLightCutoff2, cos(light->spot_cutoff * ANG2RAD) );
				glUniform1f( iLightOuterCutoff2, cos(light->spot_outer_cutoff * ANG2RAD) );
			}
			break;
		
		case 2:
			glUniform3f( iLightPos3, light_in_view_space_matrix.matrix[12], light_in_view_space_matrix.matrix[13], light_in_view_space_matrix.matrix[14] );
			glUniform1i( iLightType3, light->getLight_type() );
			glUniform4f( iLightDiffuse3, light->diffuse.x, light->diffuse.y, light->diffuse.z, light->diffuse.w );
			glUniform4f( iLightSpecular3, light->specular.x, light->specular.y, light->specular.z, light->specular.w );
			glUniform1f( iLightIntensity3, light->getIntensity() );
			if (light->getLight_type() == LSLT_SPOT) {
				glUniform3f( iLightDir3, lightDir_view_space.x, lightDir_view_space.y, lightDir_view_space.z );
				glUniform1f( iLightCutoff3, cos(light->spot_cutoff * ANG2RAD) );
				glUniform1f( iLightOuterCutoff3, cos(light->spot_outer_cutoff * ANG2RAD) );
			}
			break;
		
		default:
			// should never get here
			cout << "ERROR: More then 3 lights attached" << endl;
			break;
	}
}


