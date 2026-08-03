#ifndef LSGLTFPARSER_H
#define LSGLTFPARSER_H

// lsGLTFParser — pure-CPU glTF/.glb → engine mesh data (Plan.md Phase 7 M2).
//
// GL-free by design: no lsRender, no lsBatch_Mesh, no texture upload, so it
// can run on lsAsyncFileLoader worker threads in the streaming pipeline.
// M3 wires the output into the GL upload side (batch creation).

#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <vector>

#include "../math/lsVector3.h"
#include "../math/lsVector4.h"

/** Embedded (non-URI) image: decoded pixels + layout, as produced by
 *  tinygltf/stb_image during load. Kept as data so GL upload can happen
 *  later on the render thread. */
struct lsGLTFEmbeddedTexture
{
	std::vector<unsigned char> pixels;
	int width = 0;
	int height = 0;
	int component = 0;		///< 3 = RGB, 4 = RGBA
	int pixel_type = 0;		///< tinygltf component type (5121 = u8, 5123 = u16)
};

/** One glTF mesh primitive. Maps 1:1 to a batch on the GL upload side. */
struct lsGLTFPrimitiveData
{
	std::vector<float> positions;	///< 3 floats per vertex (required)
	std::vector<float> normals;		///< 3 floats per vertex (empty if absent)
	std::vector<float> uvs;			///< 2 floats per vertex (empty if absent)
	std::vector<float> tangents;	///< 4 floats per vertex (empty if absent)

	/** QUIRK PINNED: indices are 1-based. Legacy lsMesh::LoadGLTF adds +1
	 *  to every index (engine batch lists use slot 0 as a sentinel).
	 *  Replicated here so the M3 upload side can feed Add_Index directly. */
	std::vector<uint32_t> indices;

	lsVector4 albedo;				///< pbrMetallicRoughness.baseColorFactor
	float roughness = 1.0f;			///< pbrMetallicRoughness.roughnessFactor
	float metallic = 1.0f;			///< pbrMetallicRoughness.metallicFactor (glTF spec default = 1)

	lsGLTFPrimitiveData() : albedo( 1.0f, 1.0f, 1.0f, 1.0f ) {}

	bool hasMaterial = false;		///< false → GL side applies its own defaults (legacy behaviour)

	std::string texAlbedoUri;		///< decoded; baseDir-joined when baseDir given
	std::string texNormalUri;
	std::optional<lsGLTFEmbeddedTexture> embeddedAlbedo;
	std::optional<lsGLTFEmbeddedTexture> embeddedNormal;
};

struct lsGLTFMeshData
{
	std::vector<lsGLTFPrimitiveData> primitives;
	lsVector3 aabbMin;
	lsVector3 aabbMax;
};

/** Parse a complete glTF/.glb document from memory.
 *  @param bytes   file contents (e.g. from lsAsyncFileLoader)
 *  @param isGlb   true for binary .glb, false for ASCII .gltf
 *  @param baseDir directory of the source file — passed to tinygltf for
 *                 external-buffer resolution and prepended to texture URIs
 *  @return mesh data, or an error message. Primitives without POSITION are
 *          skipped (legacy behaviour). Non-indexed primitives get sequential
 *          (1-based) indices. Unsupported attribute/index component types
 *          are an error, not silent data corruption. */
std::expected<lsGLTFMeshData, std::string> ParseGLTF(
	const std::vector<std::byte>& bytes,
	bool isGlb,
	const std::string& baseDir = "" );

#endif // LSGLTFPARSER_H
