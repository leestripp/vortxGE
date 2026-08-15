#version 430

layout (location = 0) in vec4 vxVertex;

// per-instance data (lsBatch_Mesh::setInstanceData)
layout (location = 5) in vec4 iPosScale;  // xyz = object-local pos, w = uniform scale
layout (location = 6) in vec4 iRotMisc;   // x = cos(rotY), y = sin(rotY)

uniform mat4 mvpMatrix;

void main()
{
    float cr = iRotMisc.x;
    float sr = iRotMisc.y;
    mat3 ry = mat3( cr, 0.0, -sr,
                    0.0, 1.0, 0.0,
                    sr, 0.0, cr );
    vec3 lv = ry * ( vxVertex.xyz * iPosScale.w ) + iPosScale.xyz;
    gl_Position = mvpMatrix * vec4( lv, 1.0 );
}
