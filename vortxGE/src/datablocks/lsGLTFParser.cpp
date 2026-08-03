// lsGLTFParser — pure-CPU glTF/.glb → engine mesh data (Plan.md Phase 7 M2)
// See lsGLTFParser.h for the design contract.

#include "lsGLTFParser.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE		// matches lsMesh.cpp — stops linker errors
#include <tiny_gltf.h>

#include <algorithm>
#include <limits>
#include <numeric>

using std::string;
using std::vector;

// URI decoding, adapted from tinygltf (same helper as lsMesh.cpp; lsMesh keeps
// its own copy until M3 rewires LoadGLTF onto this parser).
namespace {
	inline unsigned char from_hex( unsigned char ch )
	{
		if (ch <= '9' && ch >= '0') ch -= '0';
		else if (ch <= 'f' && ch >= 'a') ch -= 'a' - 10;
		else if (ch <= 'F' && ch >= 'A') ch -= 'A' - 10;
		else ch = 0;
		return ch;
	}

	string urldecode( const string& str )
	{
		string result;
		for (size_t i = 0; i < str.size(); ++i) {
			if (str[i] == '+') {
				result += ' ';
			} else if (str[i] == '%' && str.size() > i + 2) {
				const unsigned char ch1 = from_hex( static_cast<unsigned char>( str[i + 1] ) );
				const unsigned char ch2 = from_hex( static_cast<unsigned char>( str[i + 2] ) );
				result += static_cast<char>( (ch1 << 4) | ch2 );
				i += 2;
			} else {
				result += str[i];
			}
		}
		return result;
	}

	// Read a float vertex attribute into `out` (floatsPerVertex * vertexCount).
	// Absent attribute → out stays empty, not an error. Non-float component
	// types (quantized attributes) are rejected rather than misread.
	bool readFloatAttribute( const tinygltf::Model& model,
							 const tinygltf::Primitive& prim,
							 const char* name,
							 int floatsPerVertex,
							 size_t vertexCount,
							 vector<float>& out,
							 string& err )
	{
		auto it = prim.attributes.find( name );
		if (it == prim.attributes.end()) return true;

		const tinygltf::Accessor& acc = model.accessors[it->second];
		if (acc.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
			err = string( "ParseGLTF: attribute " ) + name + " is not float32 (quantized attributes unsupported)";
			return false;
		}
		if (acc.count != vertexCount) {
			err = string( "ParseGLTF: attribute " ) + name + " count differs from POSITION";
			return false;
		}
		const tinygltf::BufferView& bv = model.bufferViews[acc.bufferView];
		const tinygltf::Buffer& buf = model.buffers[bv.buffer];
		int stride = acc.ByteStride( bv );
		if (stride == 0) stride = floatsPerVertex * (int)sizeof( float );

		const size_t begin = bv.byteOffset + acc.byteOffset;
		const size_t need = begin + (vertexCount ? (vertexCount - 1) * (size_t)stride : 0)
							+ (size_t)floatsPerVertex * sizeof( float );
		if (need > buf.data.size()) {
			err = string( "ParseGLTF: attribute " ) + name + " runs past its buffer";
			return false;
		}

		const unsigned char* p = buf.data.data() + begin;
		out.resize( vertexCount * (size_t)floatsPerVertex );
		for (size_t i = 0; i < vertexCount; ++i) {
			const float* f = reinterpret_cast<const float*>( p + i * (size_t)stride );
			for (int c = 0; c < floatsPerVertex; ++c)
				out[i * (size_t)floatsPerVertex + (size_t)c] = f[c];
		}
		return true;
	}
} // anonymous namespace

std::expected<lsGLTFMeshData, string> ParseGLTF(
	const vector<std::byte>& bytes,
	bool isGlb,
	const string& baseDir )
{
	if (bytes.empty())
		return std::unexpected( "ParseGLTF: empty input" );

	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	string err, warn;
	bool ok = isGlb
		? loader.LoadBinaryFromMemory( &model, &err, &warn,
			reinterpret_cast<const unsigned char*>( bytes.data() ),
			(unsigned int)bytes.size(), baseDir )
		: loader.LoadASCIIFromString( &model, &err, &warn,
			reinterpret_cast<const char*>( bytes.data() ),
			(unsigned int)bytes.size(), baseDir );
	if (!ok)
		return std::unexpected( err.empty() ? "ParseGLTF: tinygltf parse failed"
											: "ParseGLTF: " + err );

	// Resolve a texture index → URI string or embedded pixels.
	auto resolveTex = [&]( int texIndex, string& uriOut,
						   std::optional<lsGLTFEmbeddedTexture>& embeddedOut )
	{
		if (texIndex < 0 || texIndex >= (int)model.textures.size()) return;
		const tinygltf::Texture& tex = model.textures[texIndex];
		if (tex.source < 0 || tex.source >= (int)model.images.size()) return;
		const tinygltf::Image& img = model.images[tex.source];
		if (!img.uri.empty() && !tinygltf::IsDataURI( img.uri )) {
			string rel = urldecode( img.uri );
			uriOut = baseDir.empty() ? rel : baseDir + "/" + rel;
		} else if (!img.image.empty()) {
			lsGLTFEmbeddedTexture e;
			e.pixels = img.image;
			e.width = img.width;
			e.height = img.height;
			e.component = img.component;
			e.pixel_type = img.pixel_type;
			embeddedOut = std::move( e );
		}
	};

	lsGLTFMeshData mesh;
	lsVector3 min_v( std::numeric_limits<float>::max(),
					 std::numeric_limits<float>::max(),
					 std::numeric_limits<float>::max() );
	lsVector3 max_v( std::numeric_limits<float>::lowest(),
					 std::numeric_limits<float>::lowest(),
					 std::numeric_limits<float>::lowest() );
	bool havePositions = false;

	for (const auto& gm : model.meshes)
	{
		for (const auto& prim : gm.primitives)
		{
			auto posIt = prim.attributes.find( "POSITION" );
			if (posIt == prim.attributes.end()) continue;	// legacy: skip primitive

			const tinygltf::Accessor& posAcc = model.accessors[posIt->second];
			if (posAcc.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || posAcc.type != TINYGLTF_TYPE_VEC3)
				return std::unexpected( "ParseGLTF: POSITION must be float32 VEC3" );
			const size_t vertexCount = posAcc.count;

			lsGLTFPrimitiveData pd;
			string aerr;
			if (!readFloatAttribute( model, prim, "POSITION", 3, vertexCount, pd.positions, aerr ))
				return std::unexpected( aerr );
			if (!readFloatAttribute( model, prim, "NORMAL", 3, vertexCount, pd.normals, aerr ))
				return std::unexpected( aerr );
			if (!readFloatAttribute( model, prim, "TEXCOORD_0", 2, vertexCount, pd.uvs, aerr ))
				return std::unexpected( aerr );
			if (!readFloatAttribute( model, prim, "TANGENT", 4, vertexCount, pd.tangents, aerr ))
				return std::unexpected( aerr );

			for (size_t i = 0; i < vertexCount; ++i) {
				const float x = pd.positions[i * 3 + 0];
				const float y = pd.positions[i * 3 + 1];
				const float z = pd.positions[i * 3 + 2];
				min_v.x = std::min( min_v.x, x ); max_v.x = std::max( max_v.x, x );
				min_v.y = std::min( min_v.y, y ); max_v.y = std::max( max_v.y, y );
				min_v.z = std::min( min_v.z, z ); max_v.z = std::max( max_v.z, z );
			}
			havePositions = true;

			// Indices — +1 1-based quirk replicated (see header). Non-indexed
			// primitives get sequential indices per glTF draw-arrays semantics.
			if (prim.indices >= 0) {
				const tinygltf::Accessor& idxAcc = model.accessors[prim.indices];
				const tinygltf::BufferView& bv = model.bufferViews[idxAcc.bufferView];
				const tinygltf::Buffer& buf = model.buffers[bv.buffer];
				const unsigned char* p = buf.data.data() + bv.byteOffset + idxAcc.byteOffset;
				pd.indices.reserve( idxAcc.count );
				switch (idxAcc.componentType) {
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
					for (size_t i = 0; i < idxAcc.count; ++i)
						pd.indices.push_back( reinterpret_cast<const uint32_t*>( p )[i] + 1 );
					break;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
					for (size_t i = 0; i < idxAcc.count; ++i)
						pd.indices.push_back( reinterpret_cast<const uint16_t*>( p )[i] + 1 );
					break;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
					for (size_t i = 0; i < idxAcc.count; ++i)
						pd.indices.push_back( (uint32_t)p[i] + 1 );
					break;
				default:
					return std::unexpected( "ParseGLTF: unsupported index component type" );
				}
			} else {
				pd.indices.resize( vertexCount );
				std::iota( pd.indices.begin(), pd.indices.end(), 1u );	// 1-based sequential
			}

			if (prim.material >= 0 && prim.material < (int)model.materials.size()) {
				pd.hasMaterial = true;
				const tinygltf::Material& mat = model.materials[prim.material];
				const auto& pbr = mat.pbrMetallicRoughness;
				pd.albedo = lsVector4( (float)pbr.baseColorFactor[0], (float)pbr.baseColorFactor[1],
									   (float)pbr.baseColorFactor[2], (float)pbr.baseColorFactor[3] );
				pd.roughness = (float)pbr.roughnessFactor;
				pd.metallic = (float)pbr.metallicFactor;
				resolveTex( pbr.baseColorTexture.index, pd.texAlbedoUri, pd.embeddedAlbedo );
				resolveTex( mat.normalTexture.index, pd.texNormalUri, pd.embeddedNormal );
			}

			mesh.primitives.push_back( std::move( pd ) );
		}
	}

	if (havePositions) {
		mesh.aabbMin = min_v;
		mesh.aabbMax = max_v;
	} else {
		mesh.aabbMin = lsVector3( 0.0f, 0.0f, 0.0f );
		mesh.aabbMax = lsVector3( 0.0f, 0.0f, 0.0f );
	}
	return mesh;
}
