// ============================================================================
// TerrainMesh — heightfield -> engine mesh bridge (see header).
// ============================================================================

#include "TerrainMesh.h"
#include "Heightfield.h"
#include "Mesh.h"

// engine datablocks (terrain compiles into libvortxGE, so this is in-family)
#include "../datablocks/lsMesh.h"
#include "../datablocks/lsBatch_Mesh.h"
#include "../datablocks/lslistitem_vertexdata.h"
#include "../math/lsVector3.h"
#include "../math/lsBoundingSphere.h"

#include <cmath>
#include <cstring>

namespace terrain {

GridMesh generateGridMesh( const Heightfield& hf, float worldSize,
                           float maxHeight )
{
    GridMesh g;
    g.worldSize = worldSize;
    g.maxHeight = maxHeight;

    const std::size_t w = hf.width();
    const std::size_t h = hf.height();
    const std::size_t vcount = w * h;
    g.positions.resize( vcount * 3 );
    g.normals.resize( vcount * 3 );
    g.uvs.resize( vcount * 2 );
    g.indices.resize( ( w - 1 ) * ( h - 1 ) * 6 );

    const float half = worldSize * 0.5f;
    for( std::size_t y = 0; y < h; ++y )
        for( std::size_t x = 0; x < w; ++x )
        {
            const float u = static_cast<float>( x ) / static_cast<float>( w - 1 );
            const float v = static_cast<float>( y ) / static_cast<float>( h - 1 );
            const std::size_t i = y * w + x;

            g.positions[i * 3 + 0] = u * worldSize - half;
            g.positions[i * 3 + 1] = hf( x, y ) * maxHeight;
            g.positions[i * 3 + 2] = v * worldSize - half;

            // normal from the world-space gradient: n = (-dh/dx, 1, -dh/dz)
            const Vec2 grad = hf.gradient( u, v, worldSize );
            lsVector3 n( -grad.x * maxHeight, 1.0f, -grad.y * maxHeight );
            n.normalize();
            g.normals[i * 3 + 0] = n.x;
            g.normals[i * 3 + 1] = n.y;
            g.normals[i * 3 + 2] = n.z;

            g.uvs[i * 2 + 0] = u;
            g.uvs[i * 2 + 1] = v;
        }

    std::size_t t = 0;
    for( std::size_t y = 0; y + 1 < h; ++y )
        for( std::size_t x = 0; x + 1 < w; ++x )
        {
            const std::uint32_t a = static_cast<std::uint32_t>( y * w + x );
            const std::uint32_t b = a + 1;
            const std::uint32_t c = a + static_cast<std::uint32_t>( w );
            const std::uint32_t d = c + 1;
            g.indices[t++] = a; g.indices[t++] = c; g.indices[t++] = b;
            g.indices[t++] = b; g.indices[t++] = c; g.indices[t++] = d;
        }
    return g;
}

// ---------------------------------------------------------------------------
// upload helpers

static lsMesh* uploadRaw( const float* pos, const float* nrm, const float* uv,
                          std::size_t vcount, const std::uint32_t* indices,
                          std::size_t icount, lsRender* render,
                          lsShader* shader )
{
    lsBatch_Mesh* batch = new lsBatch_Mesh( render );
    if( ! batch ) return nullptr;
    batch->Init_data( static_cast<int>( vcount ), static_cast<int>( icount ) );

    for( std::size_t i = 0; i < vcount; ++i )
    {
        lsListItem_vertexdata vd;
        std::memset( &vd, 0, sizeof( vd ) );
        vd.vertex[0] = pos[i * 3 + 0];
        vd.vertex[1] = pos[i * 3 + 1];
        vd.vertex[2] = pos[i * 3 + 2];
        if( nrm )
        {
            vd.normal[0] = nrm[i * 3 + 0];
            vd.normal[1] = nrm[i * 3 + 1];
            vd.normal[2] = nrm[i * 3 + 2];
        }
        if( uv )
        {
            vd.uv[0] = uv[i * 2 + 0];
            vd.uv[1] = uv[i * 2 + 1];
        }
        batch->Add_VertexData( &vd );
    }
    for( std::size_t i = 0; i < icount; ++i )
        batch->Add_Index( static_cast<int>( indices[i] ) + 1 ); // legacy 1-based quirk

    batch->Copy_GLdata();
    if( shader ) batch->setShader( shader );

    lsMesh* mesh = new lsMesh( render );
    if( ! mesh ) return nullptr;
    mesh->Add_Batch( batch );

    // bounding sphere: centre of the AABB, radius to the far corner
    float mn[3] = { 1e30f, 1e30f, 1e30f };
    float mx[3] = { -1e30f, -1e30f, -1e30f };
    for( std::size_t i = 0; i < vcount; ++i )
        for( int a = 0; a < 3; ++a )
        {
            const float p = pos[i * 3 + a];
            if( p < mn[a] ) mn[a] = p;
            if( p > mx[a] ) mx[a] = p;
        }
    lsBoundingSphere bs;
    bs.center = lsVector3( ( mn[0] + mx[0] ) * 0.5f,
                           ( mn[1] + mx[1] ) * 0.5f,
                           ( mn[2] + mx[2] ) * 0.5f );
    bs.radius = bs.center.distance( lsVector3( mx[0], mx[1], mx[2] ) );
    mesh->setBSphere( bs );
    return mesh;
}

lsMesh* uploadGridMesh( const GridMesh& grid, lsRender* render,
                        lsShader* shader )
{
    return uploadRaw( grid.positions.data(), grid.normals.data(),
                      grid.uvs.data(), grid.positions.size() / 3,
                      grid.indices.data(), grid.indices.size(), render,
                      shader );
}

lsMesh* uploadVegetationMesh( const Mesh& mesh, lsRender* render,
                              lsShader* shader )
{
    std::vector<float> pos( mesh.vertices.size() * 3 );
    std::vector<float> nrm( mesh.vertices.size() * 3 );
    for( std::size_t i = 0; i < mesh.vertices.size(); ++i )
    {
        pos[i * 3 + 0] = mesh.vertices[i].px;
        pos[i * 3 + 1] = mesh.vertices[i].py;
        pos[i * 3 + 2] = mesh.vertices[i].pz;
        nrm[i * 3 + 0] = mesh.vertices[i].nx;
        nrm[i * 3 + 1] = mesh.vertices[i].ny;
        nrm[i * 3 + 2] = mesh.vertices[i].nz;
    }
    return uploadRaw( pos.data(), nrm.data(), nullptr, mesh.vertices.size(),
                      mesh.indices.data(), mesh.indices.size(), render,
                      shader );
}

} // namespace terrain
