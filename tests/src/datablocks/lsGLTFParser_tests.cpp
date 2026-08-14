// Tests for vortxGE/src/datablocks/lsGLTFParser.{h,cpp} (Plan.md Phase 7 M2)
// Pure CPU — no GL context needed.
//
// Coverage: all 8 demo .glb files (smoke), Suzanne.glb sanity, a hand-built
// ASCII glTF triangle (exact positions, non-indexed sequential indices,
// exact PBR factors, texture URI resolution + urldecode), and error paths.

#include "doctest/doctest.h"

#include "datablocks/lsGLTFParser.h"
#include "assets/lsAsyncFileLoader.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

using doctest::Approx;

namespace {

std::vector<std::byte> loadDemoMesh( const std::string& name )
{
	auto r = lsAsyncFileLoader::LoadFile(
		std::string( VORTXGE_SOURCE_DIR ) + "/demos/data/meshes/" + name );
	REQUIRE( r.has_value() );
	return *r;
}

// Minimal base64 encoder for building data-URI buffers in the ASCII tests.
std::string b64encode( const std::vector<unsigned char>& in )
{
	static const char* T = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	std::string out;
	for (size_t i = 0; i < in.size(); i += 3) {
		unsigned int n = (unsigned int)in[i] << 16;
		if (i + 1 < in.size()) n |= (unsigned int)in[i + 1] << 8;
		if (i + 2 < in.size()) n |= in[i + 2];
		out += T[(n >> 18) & 63];
		out += T[(n >> 12) & 63];
		out += (i + 1 < in.size()) ? T[(n >> 6) & 63] : '=';
		out += (i + 2 < in.size()) ? T[n & 63] : '=';
	}
	return out;
}

// Hand-built non-indexed ASCII glTF: one triangle (0,0,0),(1,0,0),(0,1,0)
// with a pinned PBR material and a URI base-color texture.
std::vector<std::byte> makeTriangleGLTF()
{
	const float verts[9] = { 0,0,0, 1,0,0, 0,1,0 };
	std::vector<unsigned char> bin( sizeof( verts ) );
	std::memcpy( bin.data(), verts, sizeof( verts ) );

	std::string json = std::string( R"({
  "asset": { "version": "2.0" },
  "buffers": [ { "uri": "data:application/octet-stream;base64,)" + b64encode( bin ) + R"(", "byteLength": 36 } ],
  "bufferViews": [ { "buffer": 0, "byteOffset": 0, "byteLength": 36 } ],
  "accessors": [ { "bufferView": 0, "componentType": 5126, "count": 3, "type": "VEC3",
                   "min": [0,0,0], "max": [1,1,0] } ],
  "images": [ { "uri": "textures/wood%20grain.png" } ],
  "textures": [ { "source": 0 } ],
  "materials": [ { "pbrMetallicRoughness": {
      "baseColorFactor": [0.5, 0.25, 0.75, 1.0],
      "metallicFactor": 0.1, "roughnessFactor": 0.9,
      "baseColorTexture": { "index": 0 } } } ],
  "meshes": [ { "primitives": [ { "attributes": { "POSITION": 0 }, "material": 0 } ] } ],
  "nodes": [ { "mesh": 0 } ],
  "scenes": [ { "nodes": [0] } ],
  "scene": 0
})" );
	return std::vector<std::byte>(
		reinterpret_cast<const std::byte*>( json.data() ),
		reinterpret_cast<const std::byte*>( json.data() ) + json.size() );
}

} // namespace

TEST_CASE( "lsGLTFParser: all demo .glb files parse" )
{
	const char* files[] = { "Cube.001.glb", "Cube.002.glb", "Cube.003.glb",
							"Plane.glb", "Suzanne.glb",
							"Wood_Cube.glb", "bullet_shape.glb" };
	for (const char* f : files)
	{
		CAPTURE( f );
		auto mesh = ParseGLTF( loadDemoMesh( f ), true );
		REQUIRE( mesh.has_value() );
		CHECK_FALSE( mesh->primitives.empty() );
	}
}

TEST_CASE( "lsGLTFParser: Suzanne.glb sanity" )
{
	auto mesh = ParseGLTF( loadDemoMesh( "Suzanne.glb" ), true );
	REQUIRE( mesh.has_value() );
	REQUIRE_FALSE( mesh->primitives.empty() );

	const auto& prim = mesh->primitives[0];
	CHECK( prim.positions.size() % 3 == 0 );
	const size_t vertexCount = prim.positions.size() / 3;
	CHECK( vertexCount > 100 );						// Suzanne is not a triangle

	// 1-based index quirk PINNED (legacy LoadGLTF adds +1 to every index)
	REQUIRE_FALSE( prim.indices.empty() );
	const auto [lo, hi] = std::minmax_element( prim.indices.begin(), prim.indices.end() );
	CHECK( *lo >= 1 );
	CHECK( *hi <= vertexCount );

	// Normals, when present, must match the vertex count
	if (!prim.normals.empty())
		CHECK( prim.normals.size() == prim.positions.size() );

	// AABB spans the model (Suzanne is a few units across)
	CHECK( mesh->aabbMin.x < mesh->aabbMax.x );
	CHECK( mesh->aabbMin.y < mesh->aabbMax.y );
	CHECK( mesh->aabbMax.x - mesh->aabbMin.x < 100.0f );

	// PBR factors in range
	CHECK( prim.albedo.getX() >= 0.0f );
	CHECK( prim.albedo.getX() <= 1.0f );
	CHECK( prim.roughness >= 0.0f );
	CHECK( prim.metallic >= 0.0f );
}

TEST_CASE( "lsGLTFParser: ASCII glTF triangle — exact data" )
{
	auto mesh = ParseGLTF( makeTriangleGLTF(), false, "meshes" );
	REQUIRE( mesh.has_value() );
	REQUIRE( mesh->primitives.size() == 1 );

	const auto& prim = mesh->primitives[0];
	REQUIRE( prim.positions.size() == 9 );
	CHECK( prim.positions[0] == Approx( 0.0f ) );
	CHECK( prim.positions[3] == Approx( 1.0f ) );	// v1.x
	CHECK( prim.positions[7] == Approx( 1.0f ) );	// v2.y
	CHECK( prim.normals.empty() );					// absent attribute → empty
	CHECK( prim.uvs.empty() );
	CHECK( prim.tangents.empty() );

	// Non-indexed → sequential indices, 1-based quirk PINNED
	REQUIRE( prim.indices.size() == 3 );
	CHECK( prim.indices[0] == 1u );
	CHECK( prim.indices[1] == 2u );
	CHECK( prim.indices[2] == 3u );

	CHECK( prim.hasMaterial );

	// PBR factors exact
	CHECK( prim.albedo.getX() == Approx( 0.5f ) );
	CHECK( prim.albedo.getY() == Approx( 0.25f ) );
	CHECK( prim.albedo.getZ() == Approx( 0.75f ) );
	CHECK( prim.albedo.getW() == Approx( 1.0f ) );
	CHECK( prim.roughness == Approx( 0.9f ) );
	CHECK( prim.metallic == Approx( 0.1f ) );

	// Texture URI: urldecoded + baseDir-joined
	CHECK( prim.texAlbedoUri == "meshes/textures/wood grain.png" );
	CHECK( prim.texNormalUri.empty() );
	CHECK_FALSE( prim.embeddedAlbedo.has_value() );

	// AABB exact
	CHECK( mesh->aabbMin.x == Approx( 0.0f ) );
	CHECK( mesh->aabbMax.x == Approx( 1.0f ) );
	CHECK( mesh->aabbMax.y == Approx( 1.0f ) );
	CHECK( mesh->aabbMax.z == Approx( 0.0f ) );
}

TEST_CASE( "lsGLTFParser: texture URI without baseDir stays relative" )
{
	auto mesh = ParseGLTF( makeTriangleGLTF(), false );
	REQUIRE( mesh.has_value() );
	CHECK( mesh->primitives[0].texAlbedoUri == "textures/wood grain.png" );
}

TEST_CASE( "lsGLTFParser: rejects bad input without crashing" )
{
	SUBCASE( "empty" )
	{
		CHECK_FALSE( ParseGLTF( {}, true ).has_value() );
		CHECK_FALSE( ParseGLTF( {}, false ).has_value() );
	}
	SUBCASE( "garbage as glb" )
	{
		std::vector<std::byte> junk( 64, std::byte{ 0xAB } );
		CHECK_FALSE( ParseGLTF( junk, true ).has_value() );
	}
	SUBCASE( "garbage as ascii gltf" )
	{
		const char* junk = "this is definitely not JSON {{{";
		std::vector<std::byte> bytes(
			reinterpret_cast<const std::byte*>( junk ),
			reinterpret_cast<const std::byte*>( junk ) + std::strlen( junk ) );
		CHECK_FALSE( ParseGLTF( bytes, false ).has_value() );
	}
	SUBCASE( "truncated glb header" )
	{
		auto bytes = loadDemoMesh( "Suzanne.glb" );
		bytes.resize( 8 );	// magic + version only, no length/chunks
		CHECK_FALSE( ParseGLTF( bytes, true ).has_value() );
	}
}
