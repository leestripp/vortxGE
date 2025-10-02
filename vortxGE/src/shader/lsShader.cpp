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

    shader_v = glCreateShader( GL_VERTEX_SHADER );
    shader_f = glCreateShader( GL_FRAGMENT_SHADER );
    shader = glCreateProgram();
    glAttachShader( shader, shader_v );
    glAttachShader( shader, shader_f );

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
    iMatAlbedo = -1;
    iMatRoughness = -1;
    iMatMetallic = -1;
}

lsShader::~lsShader()
{
    glDetachShader( shader, shader_v );
    glDetachShader( shader, shader_f );
    glDeleteShader( shader_v );
    glDeleteShader( shader_f );
	
    glDeleteProgram( shader );
}

void lsShader::setKind( ShaderKind val ) // Renamed from setShader_type
{
    kind = val; // Store the new kind

    // Bind shader
    glUseProgram( shader );

    // by shader kind
    switch( kind )
    {
        case ShaderKind::LightTexDNS:
            // Texture 'S' combined roughness (r), metallic (g) and AO (b).
            iTex_roughness_metallic = glGetUniformLocation( shader, "tex_roughness_metallic" );
            [[fallthrough]];
        case ShaderKind::LightTexDN:
            // Texture normal
            iTex_normal = glGetUniformLocation( shader, "tex_normal" );
            [[fallthrough]];
        case ShaderKind::LightTexD:
            // Texture 'D' Albedo
            iTex_albedo = glGetUniformLocation( shader, "tex_albedo" );
            [[fallthrough]];
        case ShaderKind::LightTexNone:
        case ShaderKind::DebugNormals: // Although debug_normals doesn't use all lights, keep light uniform init common
            // Shadow Map uniforms (arrays)
            for(int i = 0; i < 3; ++i) {
                isMatrices[i] = glGetUniformLocation( shader, ("sMatrices[" + std::to_string(i) + "]").c_str() );
                iTex_shadowmaps[i] = glGetUniformLocation( shader, ("tex_shadowmaps[" + std::to_string(i) + "]").c_str() );
                iUse_shadowmap[i] = glGetUniformLocation( shader, ("use_shadowmap[" + std::to_string(i) + "]").c_str() );
            }
            
            // Lights uniforms (arrays)
            iLightCount = glGetUniformLocation( shader, "vxLightCount" );
            for(int i = 0; i < 3; ++i) {
                iLightType[i] = glGetUniformLocation( shader, ("vxLightType[" + std::to_string(i) + "]").c_str() );
                iLightPos[i] = glGetUniformLocation( shader, ("vxLightPos[" + std::to_string(i) + "]").c_str() );
                iLightDir[i] = glGetUniformLocation( shader, ("vxLightDir[" + std::to_string(i) + "]").c_str() );
                iLightCutoff[i] = glGetUniformLocation( shader, ("vxLightCutoff[" + std::to_string(i) + "]").c_str() );
                iLightOuterCutoff[i] = glGetUniformLocation( shader, ("vxLightOuterCutoff[" + std::to_string(i) + "]").c_str() );
                iLightDiffuse[i] = glGetUniformLocation( shader, ("vxLightDiffuse[" + std::to_string(i) + "]").c_str() );
                iLightSpecular[i] = glGetUniformLocation( shader, ("vxLightSpecular[" + std::to_string(i) + "]").c_str() );
                iLightIntensity[i] = glGetUniformLocation( shader, ("vxLightIntensity[" + std::to_string(i) + "]").c_str() );
            }
            
            // Material
            iMatAlbedo = glGetUniformLocation( shader, "vxMatAlbedo" );
            iMatRoughness = glGetUniformLocation( shader, "vxMatRoughness" );
            iMatMetallic = glGetUniformLocation( shader, "vxMatMetallic" );
            
            // Need ModelViewMatrix and ProjectionMatrix
            imvMatrix = glGetUniformLocation( shader, "mvMatrix" );
            imMatrix = glGetUniformLocation( shader, "mMatrix" );
            ipMatrix = glGetUniformLocation( shader, "pMatrix" );
            // Normal Matrix
            inMatrix = glGetUniformLocation( shader, "nMatrix" );
            break;

        case ShaderKind::Quad:
            // Texture
            iTex_albedo = glGetUniformLocation( shader, "tex_albedo" );
            [[fallthrough]];
        case ShaderKind::Debug:
            // Need ModelViewMatrix and ProjectionMatrix
            imvpMatrix = glGetUniformLocation( shader, "mvpMatrix" );
            break;

        case ShaderKind::ShadowMap:
            imvpMatrix = glGetUniformLocation( shader, "mvpMatrix" );
            break;

        case ShaderKind::Skybox: // Initialize uniforms for skybox shader
            ipMatrix = glGetUniformLocation(shader, "pMatrix");
            imvMatrix = glGetUniformLocation(shader, "mvMatrix");
            iTex_albedo = glGetUniformLocation(shader, "tex_skybox");
            break;
            
        default:
            break;
    } // switch kind
    
    glUseProgram(0);
}

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

    glUseProgram( shader );
    
    switch( kind ) // Changed from shader_type to kind
    {
        case ShaderKind::LightTexDNS:
            // Texture 'S' combined roughness (r), metallic (g) and AO (b).
            glActiveTexture( GL_TEXTURE2 );
            glBindTexture( GL_TEXTURE_2D, batch_mesh->tex_roughness_metallic );
            glUniform1i( iTex_roughness_metallic, 2 );
            [[fallthrough]];
        case ShaderKind::LightTexDN:
            // Texture normal
            glActiveTexture( GL_TEXTURE1 );
            glBindTexture( GL_TEXTURE_2D, batch_mesh->tex_normal );
            glUniform1i( iTex_normal, 1 );
            [[fallthrough]];
        case ShaderKind::LightTexD:
            // Texture 'D' Albedo
            glActiveTexture( GL_TEXTURE0 );
            glBindTexture( GL_TEXTURE_2D, batch_mesh->tex_albedo );
            glUniform1i( iTex_albedo, 0 );
            [[fallthrough]];
        case ShaderKind::LightTexNone:
            // Shadow Map & Light Uniforms
            {
                const auto& lights = object->getLightManager()->getLights();
                int active_lights_count = 0;
                for (int i = 0; i < std::min((int)lights.size(), 3); ++i)
                {
                    lsLight* light = lights[i];
                    if (light)
                    {
                        // Bind shadow map texture for this light
                        if (light->getHas_shadowmap())
                        {
                            glActiveTexture(GL_TEXTURE3 + i); // Use texture units 3, 4, 5 for shadow maps
                            glBindTexture(GL_TEXTURE_2D, light->getTex_shadowmap());
                            glUniform1i(iTex_shadowmaps[i], 3 + i);
                            glUniform1i(iUse_shadowmap[i], 1);
                            glUniformMatrix4fv(isMatrices[i], 1, GL_FALSE, light->ShadowmapMatrix.matrix);
                        } else {
                            glUniform1i(iUse_shadowmap[i], 0);
                        }
                        active_lights_count++; // Count actual lights processed
                    }
                }
                // Pass the count of active lights to the shader
                glUniform1i(iLightCount, active_lights_count);
                Setup_Lights(cam, object); // Now Setup_Lights will pass all light properties
            }
            
            // Material
            if( batch->getBatch_type() == LSBT_MESH )
            {
                glUniform4f( iMatAlbedo, batch_mesh->m_albedo.x, batch_mesh->m_albedo.y, batch_mesh->m_albedo.z, batch_mesh->m_albedo.w );
                glUniform1f( iMatRoughness, batch_mesh->m_roughness );
                glUniform1f( iMatMetallic, batch_mesh->m_metallic );
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

        case ShaderKind::DebugNormals:
            // Material
            if( batch->getBatch_type() == LSBT_MESH )
            {
                glUniform4f( iMatAlbedo, batch_mesh->m_albedo.x, batch_mesh->m_albedo.y, batch_mesh->m_albedo.z, batch_mesh->m_albedo.w );
                glUniform1f( iMatRoughness, batch_mesh->m_roughness );
                glUniform1f( iMatMetallic, batch_mesh->m_metallic );
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

        case ShaderKind::Quad:
            pMatrix = cam->frustum.getProjectionMatrix();
            mvMatrix = cam->ViewMatrix * object->ModelMatrix;
            mvpMatrix = pMatrix * mvMatrix;
            glUniformMatrix4fv( imvpMatrix, 1, GL_FALSE, mvpMatrix.matrix );
            
            // Textures
            glActiveTexture( GL_TEXTURE0 );
            glBindTexture( GL_TEXTURE_2D, batch_quad->getTex_albedo() );
            glUniform1i( iTex_albedo, 0 );
            break;
        
        case ShaderKind::Debug:
            pMatrix = cam->frustum.getProjectionMatrix();
            mvMatrix = cam->ViewMatrix * object->ModelMatrix;
            mvpMatrix = pMatrix * mvMatrix;
            glUniformMatrix4fv( imvpMatrix, 1, GL_FALSE, mvpMatrix.matrix );
            break;

        case ShaderKind::ShadowMap:
            pMatrix = cam->frustum.getProjectionMatrix();
            mvMatrix = cam->ViewMatrix * object->ModelMatrix;
            mvpMatrix = pMatrix * mvMatrix;
            glUniformMatrix4fv(imvpMatrix, 1, GL_FALSE, mvpMatrix.matrix);
            break;

        case ShaderKind::Skybox:
            pMatrix = cam->frustum.getProjectionMatrix();
            // For skybox, remove translation from camera's view matrix
            // This keeps the skybox centered on the camera, giving infinite distance
            mvMatrix = cam->ViewMatrix.extractRotation(); 
            
            glUniformMatrix4fv(ipMatrix, 1, GL_FALSE, pMatrix.matrix);
            glUniformMatrix4fv(imvMatrix, 1, GL_FALSE, mvMatrix.matrix);

            // Bind skybox texture
            glActiveTexture(GL_TEXTURE0);
            // Assuming tex_albedo holds the skybox texture, cast batch to lsBatch_Mesh
            if (batch->getBatch_type() == LSBT_MESH) {
                lsBatch_Mesh* mesh_batch = static_cast<lsBatch_Mesh*>(batch);
                glBindTexture(GL_TEXTURE_2D, mesh_batch->tex_albedo); 
            }
            glUniform1i(iTex_albedo, 0);
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
    int num_lights = std::min((int)lights.size(), 3); // Shader supports max 3 lights
    
    for (int i = 0; i < num_lights; ++i)
    {
        lsLight* light = lights[i];
        if (light)
        {
            Set_Light(i, camera, light, object);
        }
    }
    
    // Default remaining lights to type 0 (LSLT_NONE) to disable them in shader
    for(int i = num_lights; i < 3; ++i) {
        if (iLightType[i] != -1) glUniform1i(iLightType[i], LSLT_NONE);
    }

    glUniform1i(iLightCount, num_lights);
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
    
    if (iLightPos[i] != -1) glUniform3f( iLightPos[i], light_in_view_space_matrix.matrix[12], light_in_view_space_matrix.matrix[13], light_in_view_space_matrix.matrix[14] );
    if (iLightType[i] != -1) glUniform1i( iLightType[i], light->getLight_type() );
    if (iLightDiffuse[i] != -1) glUniform4f( iLightDiffuse[i], light->diffuse.x, light->diffuse.y, light->diffuse.z, light->diffuse.w );
    if (iLightSpecular[i] != -1) glUniform4f( iLightSpecular[i], light->specular.x, light->specular.y, light->specular.z, light->specular.w );
    if (iLightIntensity[i] != -1) glUniform1f( iLightIntensity[i], light->getIntensity() );

    // Only set direction and cutoff for spot/direct lights
    if (light->getLight_type() == LSLT_SPOT || light->getLight_type() == LSLT_DIRECT)
    {
        if (iLightDir[i] != -1) glUniform3f( iLightDir[i], lightDir_view_space.x, lightDir_view_space.y, lightDir_view_space.z );
        if (iLightCutoff[i] != -1) glUniform1f( iLightCutoff[i], cos(light->spot_cutoff * ANG2RAD) );
        if (iLightOuterCutoff[i] != -1) glUniform1f( iLightOuterCutoff[i], cos(light->spot_outer_cutoff * ANG2RAD) );
    }
}
