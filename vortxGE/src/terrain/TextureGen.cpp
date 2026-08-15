#include "TextureGen.h"
#include "Noise.h"

#include <algorithm>
#include <cmath>

namespace terrain {

namespace {

// Seamlessly tiling fBm: sample the field at the 4 wrapped offsets and
// bilinearly crossfade so the right/bottom edges meet the left/top.
float tileFbm( const Noise& n, float x, float y, float period,
               int octaves, float fx, float fy )
{
    const float a = n.fbm( x,         y,         octaves );
    const float b = n.fbm( x - period, y,         octaves );
    const float c = n.fbm( x,         y - period, octaves );
    const float d = n.fbm( x - period, y - period, octaves );
    const float top    = a + ( b - a ) * fx;
    const float bottom = c + ( d - c ) * fx;
    return top + ( bottom - top ) * fy;
}

std::uint8_t to8( float v )
{
    return ( std::uint8_t )std::clamp( v * 255.0f, 0.0f, 255.0f );
}

} // namespace

SplatLayer generateSplatLayer( int kind, int size )
{
    SplatLayer layer;
    layer.size = size;
    layer.rgba.resize( ( size_t )size * size * 4 );

    // Per-layer seeds so the patterns are decorrelated.
    Noise base( 0xC0FFEE + ( std::uint64_t )kind * 7919 );
    Noise fine( 0xBADD00 + ( std::uint64_t )kind * 104729 );

    // Periods in noise-lattice units per texture tile (integers = the
    // wrap-blend above lines up exactly).
    const float P  = 8.0f;   // large structure
    const float P2 = 32.0f;  // fine grain

    for( int y = 0; y < size; ++y )
    {
        const float fy = ( float )y / size;
        for( int x = 0; x < size; ++x )
        {
            const float fx = ( float )x / size;
            const float nx = fx * P,  ny = fy * P;
            const float gx = fx * P2, gy = fy * P2;

            // large blotch field [-1,1] and fine grain [-1,1], tiling
            const float large = tileFbm( base, nx, ny, P,  4, fx, fy );
            const float grain = tileFbm( fine, gx, gy, P2, 3, fx, fy );

            float r = 0, g = 0, b = 0, h = 0;
            switch( kind )
            {
            case 0: // grass: green, hue/light vary with blotch, blade grain
            {
                const float l = 0.80f + 0.20f * large + 0.10f * grain;
                r = 0.35f * l; g = 0.70f * l; b = 0.22f * l;
                h = 0.45f + 0.20f * grain;            // blades ~ mid height
                break;
            }
            case 1: // dirt: brown, patchy lumps
            {
                const float lump = tileFbm( base, nx * 2.0f, ny * 2.0f, P * 2.0f, 4, fx, fy );
                const float l = 0.70f + 0.18f * large + 0.12f * lump;
                r = 0.55f * l; g = 0.40f * l; b = 0.26f * l;
                h = 0.40f + 0.25f * lump;             // clods
                break;
            }
            case 2: // rock: grey strata, ridged relief — dominates height blend
            {
                const float ridge = 1.0f - std::fabs( tileFbm( base, nx * 1.5f, ny * 1.5f, P * 1.5f, 5, fx, fy ) );
                const float l = 0.55f + 0.25f * ridge + 0.08f * grain;
                r = g = b = l;
                h = 0.55f + 0.45f * ridge;            // stones sit proud
                break;
            }
            default: // sand: pale, smooth dunes + fine ripple
            {
                const float ripple = std::sin( ( fx * P2 + large * 2.0f ) * 3.14159f );
                const float l = 0.85f + 0.10f * large + 0.05f * ripple;
                r = 0.85f * l; g = 0.75f * l; b = 0.55f * l;
                h = 0.30f + 0.10f * large;            // lies low, fills cracks
                break;
            }
            }

            const size_t i = ( ( size_t )y * size + x ) * 4;
            layer.rgba[i + 0] = to8( r );
            layer.rgba[i + 1] = to8( g );
            layer.rgba[i + 2] = to8( b );
            layer.rgba[i + 3] = to8( std::clamp( h, 0.0f, 1.0f ) );
        }
    }
    return layer;
}

} // namespace terrain
