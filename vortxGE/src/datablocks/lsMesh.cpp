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

// tinygltf
#define TINYGLTF_NO_STB_IMAGE_WRITE		// Needed to stop linker errors.
#include <tiny_gltf.h>
#include <limits>

// Helper functions for URI decoding, adapted from tinygltf
namespace {
	inline unsigned char from_hex(unsigned char ch)
	{
		if (ch <= '9' && ch >= '0')
			ch -= '0';
		else if (ch <= 'f' && ch >= 'a')
			ch -= 'a' - 10;
		else if (ch <= 'F' && ch >= 'A')
			ch -= 'A' - 10;
		else
			ch = 0;
		return ch;
	}

	static string urldecode(const string &str)
	{
	  using namespace std;
	  string result;
	  string::size_type i;
	  for (i = 0; i < str.size(); ++i) {
		if (str[i] == '+') {
		  result += ' ';
		} else if (str[i] == '%' && str.size() > i + 2) {
		  const unsigned char ch1 =
			  from_hex(static_cast<unsigned char>(str[i + 1]));
		  const unsigned char ch2 =
			  from_hex(static_cast<unsigned char>(str[i + 2]));
		  const unsigned char ch = static_cast<unsigned char>((ch1 << 4) | ch2);
		  result += static_cast<char>(ch);
		  i += 2;
		} else {
		  result += str[i];
		}
	  }
	  return result;
	}
} // anonymous namespace


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
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	string err;
	string warn;

	// Get executable path.
	lsDir dir;
	string basepath = dir.getexepath();
	
	string ext;
	size_t pos = filename.find_last_of(".");
	if (pos != string::npos) {
		ext = filename.substr(pos + 1);
	}
	
	bool ret = (ext == "glb") ? loader.LoadBinaryFromFile(&model, &err, &warn, basepath + filename)
							  : loader.LoadASCIIFromFile(&model, &err, &warn, basepath + filename);

	if (!warn.empty()) { cout << "glTF WARN: " << warn << endl; }
	if (!err.empty()) { cerr << "glTF ERR: " << err << endl; }
	if (!ret) { cerr << "Failed to load glTF: " << filename << endl; return; }

	lsVector3 min_v(numeric_limits<float>::max(), numeric_limits<float>::max(), numeric_limits<float>::max());
	lsVector3 max_v(numeric_limits<float>::lowest(), numeric_limits<float>::lowest(), numeric_limits<float>::lowest());
	vector<lsVector3> all_vertices;

	for (const auto& mesh : model.meshes) {
		for (const auto& primitive : mesh.primitives) {
			lsBatch_Mesh* batch = new lsBatch_Mesh(getRender());
			if (!batch) continue;

			// Attributes
			vector<lsListItem_vertexdata> vertexData;
			size_t vertexCount = 0;

			if (primitive.attributes.find("POSITION") == primitive.attributes.end()) {
				delete batch;
				continue;
			}
			
			const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
			vertexCount = posAccessor.count;
			vertexData.resize(vertexCount);
			
			{ // Position
				const tinygltf::BufferView& bufferView = model.bufferViews[posAccessor.bufferView];
				const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
				const unsigned char* data_ptr = &buffer.data[bufferView.byteOffset + posAccessor.byteOffset];
				int stride = posAccessor.ByteStride(bufferView);
				if (stride == 0) stride = 3 * sizeof(float);

				for (size_t i = 0; i < vertexCount; ++i) {
					const float* p = reinterpret_cast<const float*>(data_ptr + i * stride);
					vertexData[i].vertex[0] = p[0];
					vertexData[i].vertex[1] = p[1];
					vertexData[i].vertex[2] = p[2];

					min_v.x = min(min_v.x, p[0]);
					min_v.y = min(min_v.y, p[1]);
					min_v.z = min(min_v.z, p[2]);
					max_v.x = max(max_v.x, p[0]);
					max_v.y = max(max_v.y, p[1]);
					max_v.z = max(max_v.z, p[2]);
					all_vertices.emplace_back(p[0], p[1], p[2]);
				}
			}
			
			if (primitive.attributes.count("NORMAL")) {
				const auto& accessor = model.accessors[primitive.attributes.at("NORMAL")];
				const auto& bufferView = model.bufferViews[accessor.bufferView];
				const auto& buffer = model.buffers[bufferView.buffer];
				const unsigned char* data_ptr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
				int stride = accessor.ByteStride(bufferView);
				if (stride == 0) stride = 3 * sizeof(float);
				for (size_t i = 0; i < vertexCount; ++i) {
					const float* p = reinterpret_cast<const float*>(data_ptr + i * stride);
					vertexData[i].normal[0] = p[0];
					vertexData[i].normal[1] = p[1];
					vertexData[i].normal[2] = p[2];
				}
			}
			
			if (primitive.attributes.count("TEXCOORD_0")) {
				const auto& accessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
				const auto& bufferView = model.bufferViews[accessor.bufferView];
				const auto& buffer = model.buffers[bufferView.buffer];
				const unsigned char* data_ptr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
				int stride = accessor.ByteStride(bufferView);
				if (stride == 0) stride = 2 * sizeof(float);
				for (size_t i = 0; i < vertexCount; ++i) {
					const float* p = reinterpret_cast<const float*>(data_ptr + i * stride);
					vertexData[i].uv[0] = p[0];
					vertexData[i].uv[1] = p[1];
				}
			}

			if (primitive.attributes.count("TANGENT")) {
				const auto& accessor = model.accessors[primitive.attributes.at("TANGENT")];
				const auto& bufferView = model.bufferViews[accessor.bufferView];
				const auto& buffer = model.buffers[bufferView.buffer];
				const unsigned char* data_ptr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
				int stride = accessor.ByteStride(bufferView);
				if (stride == 0) stride = 4 * sizeof(float);
				for (size_t i = 0; i < vertexCount; ++i) {
					const float* p = reinterpret_cast<const float*>(data_ptr + i * stride);
					vertexData[i].tangent[0] = p[0];
					vertexData[i].tangent[1] = p[1];
					vertexData[i].tangent[2] = p[2];
				}
			}

			// Indices
			const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
			const tinygltf::BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
			const tinygltf::Buffer& indexBuffer = model.buffers[indexBufferView.buffer];
			const unsigned char* indexData = &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset];
			
			batch->Init_data(vertexCount, indexAccessor.count);
			for(const auto& vd : vertexData) {
				lsListItem_vertexdata temp_vd = vd;
				batch->Add_VertexData(&temp_vd);
			}
			
			if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
				const uint32_t* p = reinterpret_cast<const uint32_t*>(indexData);
				for (size_t i = 0; i < indexAccessor.count; ++i) batch->Add_Index(p[i] + 1);
			} else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
				const uint16_t* p = reinterpret_cast<const uint16_t*>(indexData);
				for (size_t i = 0; i < indexAccessor.count; ++i) batch->Add_Index(p[i] + 1);
			} else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
				const uint8_t* p = reinterpret_cast<const uint8_t*>(indexData);
				for (size_t i = 0; i < indexAccessor.count; ++i) batch->Add_Index(p[i] + 1);
			}

			batch->Copy_GLdata();
			addMem_used(batch->getMem_used());

			if (primitive.material >= 0) {
				const tinygltf::Material& mat = model.materials[primitive.material];
				
				const auto& pbr = mat.pbrMetallicRoughness;
				batch->m_albedo = lsVector4(pbr.baseColorFactor[0], pbr.baseColorFactor[1], pbr.baseColorFactor[2], pbr.baseColorFactor[3]);
				batch->m_roughness = pbr.roughnessFactor;
				batch->m_metallic = pbr.metallicFactor;
				
				string shader_name_suffix = "";

				auto get_texture_id = [&](int tex_idx) -> lsTextureID {
					if (tex_idx < 0 || static_cast<size_t>(tex_idx) >= model.textures.size()) return 0;
					const auto& tex = model.textures[tex_idx];
					if (tex.source < 0 || static_cast<size_t>(tex.source) >= model.images.size()) return 0;
					const auto& img = model.images[tex.source];
					
					if (!img.uri.empty() && !tinygltf::IsDataURI(img.uri)) {
						string base_dir = filename.substr(0, filename.find_last_of("/\\"));
						string tex_path = base_dir + "/" + urldecode(img.uri);

						lsAsset_Texture *image_asset = (lsAsset_Texture *)am->Find_Asset(tex_path);
						if (!image_asset) {
							image_asset = am->CreateAsset_Texture(tex_path);
							image_asset->AllocAsset();
						}
						if (image_asset && image_asset->getData()) {
							return static_cast<lsTexture*>(image_asset->getData())->getTexture();
						}
					} else if (!img.image.empty()) {
						lsTextureID tex_id = 0;
						getRender()->GenTextures(1, &tex_id);
						getRender()->BindTexture(LS_TEXTURE_2D, tex_id);
						lsEnum format = (img.component == 4) ? LS_RGBA : LS_RGB;
						
						if (img.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
							lsEnum internal_format = (img.component == 4) ? LS_RGBA16 : LS_RGB16;
							getRender()->TexImage2D(LS_TEXTURE_2D, 0, internal_format, img.width, img.height, 0, format, LS_UNSIGNED_SHORT, img.image.data());
						} else {
							lsEnum internal_format = (img.component == 4) ? LS_SRGB8_ALPHA8 : LS_SRGB8;
							getRender()->TexImage2D(LS_TEXTURE_2D, 0, internal_format, img.width, img.height, 0, format, LS_UNSIGNED_BYTE, img.image.data());
						}
						
						getRender()->GenerateMipmap(LS_TEXTURE_2D);
						getRender()->TexParameteri(LS_TEXTURE_2D, LS_TEXTURE_WRAP_S, LS_REPEAT);
						getRender()->TexParameteri(LS_TEXTURE_2D, LS_TEXTURE_WRAP_T, LS_REPEAT);
						getRender()->TexParameteri(LS_TEXTURE_2D, LS_TEXTURE_MIN_FILTER, LS_LINEAR_MIPMAP_LINEAR);
						getRender()->TexParameteri(LS_TEXTURE_2D, LS_TEXTURE_MAG_FILTER, LS_LINEAR);
						getRender()->BindTexture(LS_TEXTURE_2D, 0);
						return tex_id;
					}
					return 0;
				};

				if (pbr.baseColorTexture.index > -1) {
					batch->tex_albedo = get_texture_id(pbr.baseColorTexture.index);
					if (batch->tex_albedo != 0) shader_name_suffix += "d";
				}
				if (mat.normalTexture.index > -1) {
					batch->tex_normal = get_texture_id(mat.normalTexture.index);
					if (batch->tex_normal != 0) shader_name_suffix += "n";
				}
				
				string shader_name = "light_tex_";
				if (shader_name_suffix.empty()) {
					shader_name += "none";
				} else {
					shader_name += shader_name_suffix;
				}
				
				lsShader* shader = sm->Find_Shader(shader_name);
				if (!shader) {
					shader = new lsShader(getRender());
					shader->Load_Shader(shader_name);
					if(shader->getKind() != ShaderKind::None)
					{
						sm->Add_Shader(shader);
					} else
					{
						delete shader;
						shader = sm->Find_Shader("light_tex_none");
					}
				}
				batch->setShader(shader);
			} else
			{
				 lsShader* shader = sm->Find_Shader("light_tex_none");
				 batch->setShader(shader);
			}

			Add_Batch(batch);
		}
	}

	// Set bounding sphere.
	if (! all_vertices.empty())
	{
		bSphere.center = (min_v + max_v) * 0.5f;
		bSphere.radius = 0.0f;
		for(const auto& vert : all_vertices)
		{
			float dist = (vert - bSphere.center).length();
			if(dist > bSphere.radius)
			{
				bSphere.radius = dist;
			}
		}
	} else
	{
		bSphere.center = lsVector3(0,0,0);
		bSphere.radius = 0.0f;
	}
}

