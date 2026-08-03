#include <iostream>
// debug
#include "../vortx_debug.h"

// OpenGL
#include "../render/lsRender.h"

// filesystem
#include "../filesystem/lsFileIO.h"
#include "../filesystem/lsDir.h"

// shaderss
#include "../shader/lsShaderManager.h"
#include "../shader/lsShader.h"

// assets
#include "../assets/lsAssetManager.h"
#include "../assets/lsAsset_Mesh.h"
#include "../assets/lsAsset_Texture.h"

// scene
#include "../scene/lsNode.h"
#include "../scene/lsObject.h"

// animation
#include "../animation/lsAnimationManager.h"

// shader
#include "../shader/lsShader.h"

// datablocks
#include "lsString.h"
#include "lsBatch.h"
#include "lsBatch_Lines.h"
#include "lsBatch_Mesh.h"
#include "lsBatch_Normals.h"
#include "lsTexture.h"
#include "lslinkedlist.h"
#include "lslistitem_vector2.h"
#include "lslistitem_vector3.h"
#include "lslistitem_index.h"
#include "lslistitem_facetri.h"
#include "lslistitem_facequad.h"
#include "lslistitem_vertexdata.h"
#include "lsMesh.h"

#include "lsGLTFParser.h"
#include "../assets/lsAsyncFileLoader.h"
#include <optional>

// tinygltf (kept for the UNSIGNED_SHORT pixel-type constant in the embedded
// texture upload; full decoupling lands with M3)
#define TINYGLTF_NO_STB_IMAGE_WRITE		// Needed to stop linker errors.
#include <tiny_gltf.h>
#include <limits>



lsMesh::lsMesh( lsRender *render ) : lsData()
{
	m_render = render;
	batch_list = nullptr;
}

lsMesh::~lsMesh()
{
	// lsBatch auto deletes
	if( batch_list ) delete batch_list;
}

void lsMesh::Draw( lsCamera *cam, lsPass *pass, lsObject *object )
{
	if( batch_list ) batch_list->Draw( cam, object, pass );
}

void lsMesh::Add_Batch( lsBatch *batch )
{
#ifdef VORTX_DEBUG_BATCH
	cout << "lsMesh::Add_Batch() - Name: " << batch->getName() << endl;
#endif

	batch->setNext_batch( batch_list );
	if( batch_list ) batch_list->setPrev_batch( batch );
	batch_list = batch;
}

lsRender *lsMesh::getRender() const
{
	return m_render;
}

void lsMesh::setOgl(lsRender *value)
{
	m_render = value;
}

void lsMesh::Update_Animation()
{
	// if( batch_list ) batch_list->Update();
}



void lsMesh::LoadGLTF(const string& filename, lsAssetManager *am, lsShaderManager *sm)
{
	// M2: CPU parse is GL-free (lsGLTFParser); this function is the GL upload
	// side — batches, textures, shaders, bounding sphere.

	// Get executable path.
	lsDir dir;
	string basepath = dir.getexepath();

	string ext;
	size_t pos = filename.find_last_of(".");
	if (pos != string::npos) {
		ext = filename.substr(pos + 1);
	}

	auto fileBytes = lsAsyncFileLoader::LoadFile( basepath + filename );
	if (!fileBytes) {
		cerr << "Failed to load glTF: " << filename << " (" << fileBytes.error() << ")" << endl;
		return;
	}

	size_t slash = filename.find_last_of("/\\");
	string baseDir = (slash == string::npos) ? "" : filename.substr(0, slash);

	auto parsed = ParseGLTF( *fileBytes, ext == "glb", baseDir );
	if (!parsed) {
		cerr << "Failed to load glTF: " << filename << " (" << parsed.error() << ")" << endl;
		return;
	}

	UploadGLTF( *parsed, am, sm );
}

void lsMesh::UploadGLTF( const lsGLTFMeshData& data, lsAssetManager *am, lsShaderManager *sm )
{
	const lsGLTFMeshData* parsed = &data;

	for (const auto& prim : parsed->primitives)
	{
		lsBatch_Mesh* batch = new lsBatch_Mesh(getRender());
		if (!batch) continue;

		const size_t vertexCount = prim.positions.size() / 3;
		batch->Init_data(vertexCount, prim.indices.size());

		for (size_t i = 0; i < vertexCount; ++i)
		{
			lsListItem_vertexdata vd;
			vd.vertex[0] = prim.positions[i * 3 + 0];
			vd.vertex[1] = prim.positions[i * 3 + 1];
			vd.vertex[2] = prim.positions[i * 3 + 2];
			if (!prim.normals.empty()) {
				vd.normal[0] = prim.normals[i * 3 + 0];
				vd.normal[1] = prim.normals[i * 3 + 1];
				vd.normal[2] = prim.normals[i * 3 + 2];
			}
			if (!prim.uvs.empty()) {
				vd.uv[0] = prim.uvs[i * 2 + 0];
				vd.uv[1] = prim.uvs[i * 2 + 1];
			}
			if (!prim.tangents.empty()) {
				vd.tangent[0] = prim.tangents[i * 4 + 0];
				vd.tangent[1] = prim.tangents[i * 4 + 1];
				vd.tangent[2] = prim.tangents[i * 4 + 2];
			}
			batch->Add_VertexData(&vd);
		}
		for (uint32_t idx : prim.indices)
			batch->Add_Index( idx );	// already 1-based (parser replicates legacy quirk)

		batch->Copy_GLdata();
		addMem_used(batch->getMem_used());

		// Texture upload — URI assets via the asset manager (dedup via
		// Find_Asset), embedded pixels direct to GL. Returns 0 on failure.
		auto upload_texture = [&]( const string& uri,
								   const optional<lsGLTFEmbeddedTexture>& emb ) -> lsTextureID
		{
			if (!uri.empty()) {
				lsAsset_Texture *image_asset = (lsAsset_Texture *)am->Find_Asset( uri );
				if (!image_asset) {
					image_asset = am->CreateAsset_Texture( uri );
					image_asset->AllocAsset();
				}
				if (image_asset && image_asset->getData()) {
					return static_cast<lsTexture*>(image_asset->getData())->getTexture();
				}
				return 0;
			}
			if (emb && !emb->pixels.empty()) {
				lsTextureID tex_id = 0;
				getRender()->GenTextures( 1, &tex_id );
				getRender()->BindTexture( LS_TEXTURE_2D, tex_id );
				lsEnum format = (emb->component == 4) ? LS_RGBA : LS_RGB;
				if (emb->pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
					lsEnum internal_format = (emb->component == 4) ? LS_RGBA16 : LS_RGB16;
					getRender()->TexImage2D( LS_TEXTURE_2D, 0, internal_format, emb->width, emb->height, 0, format, LS_UNSIGNED_SHORT, emb->pixels.data() );
				} else {
					lsEnum internal_format = (emb->component == 4) ? LS_SRGB8_ALPHA8 : LS_SRGB8;
					getRender()->TexImage2D( LS_TEXTURE_2D, 0, internal_format, emb->width, emb->height, 0, format, LS_UNSIGNED_BYTE, emb->pixels.data() );
				}
				getRender()->GenerateMipmap( LS_TEXTURE_2D );
				getRender()->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_WRAP_S, LS_REPEAT );
				getRender()->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_WRAP_T, LS_REPEAT );
				getRender()->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_MIN_FILTER, LS_LINEAR_MIPMAP_LINEAR );
				getRender()->TexParameteri( LS_TEXTURE_2D, LS_TEXTURE_MAG_FILTER, LS_LINEAR );
				getRender()->BindTexture( LS_TEXTURE_2D, 0 );
				return tex_id;
			}
			return 0;
		};

		if (prim.hasMaterial)
		{
			batch->m_albedo = prim.albedo;
			batch->m_roughness = prim.roughness;
			batch->m_metallic = prim.metallic;

			string shader_name_suffix = "";
			if (!prim.texAlbedoUri.empty() || prim.embeddedAlbedo) {
				batch->tex_albedo = upload_texture( prim.texAlbedoUri, prim.embeddedAlbedo );
				if (batch->tex_albedo != 0) shader_name_suffix += "d";
			}
			if (!prim.texNormalUri.empty() || prim.embeddedNormal) {
				batch->tex_normal = upload_texture( prim.texNormalUri, prim.embeddedNormal );
				if (batch->tex_normal != 0) shader_name_suffix += "n";
			}
			string shader_name = "light_tex_";
			shader_name += shader_name_suffix.empty() ? "none" : shader_name_suffix;
			lsShader* shader = sm->Find_Shader( shader_name );
			if (!shader) {
				shader = new lsShader(getRender());
				shader->Load_Shader( shader_name );
				if (shader->getKind() != ShaderKind::None) {
					sm->Add_Shader( shader );
				} else {
					delete shader;
					shader = sm->Find_Shader( "light_tex_none" );
				}
			}
			batch->setShader( shader );
		} else {
			lsShader* shader = sm->Find_Shader( "light_tex_none" );
			batch->setShader( shader );
		}
		Add_Batch( batch );
	}

	// Bounding sphere over all parsed positions. Degenerates to the legacy
	// empty-mesh values (centre 0, radius 0) when nothing was parsed.
	bSphere.center = (parsed->aabbMin + parsed->aabbMax) * 0.5f;
	bSphere.radius = 0.0f;
	for (const auto& prim : parsed->primitives) {
		for (size_t i = 0; i + 2 < prim.positions.size(); i += 3) {
			lsVector3 vert( prim.positions[i], prim.positions[i + 1], prim.positions[i + 2] );
			float dist = (vert - bSphere.center).length();
			if (dist > bSphere.radius) bSphere.radius = dist;
		}
	}
}

