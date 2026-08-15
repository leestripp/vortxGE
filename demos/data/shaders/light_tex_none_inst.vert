#version 430

// vbo data
layout (location = 0) in vec4 vxVertex;
layout (location = 1) in vec4 vxColour;
layout (location = 2) in vec3 vxNormal;
layout (location = 3) in vec3 vxTangent;
layout (location = 4) in vec2 vxTexCoord;

// per-instance data (lsBatch_Mesh::setInstanceData, 8 floats/instance)
layout (location = 5) in vec4 iPosScale;  // xyz = object-local pos, w = uniform scale
layout (location = 6) in vec4 iRotMisc;   // x = cos(rotY), y = sin(rotY), z = colour var, w = pad

// Uniforms
uniform mat4 pMatrix;
uniform mat4 mvMatrix;
uniform mat4 mMatrix; // Model Matrix
uniform mat4 sMatrices[3]; // Changed to array
// Normal Matrix
uniform mat3 nMatrix;

// outputs
out vec3 osVertex;
out vec4 osColour;
out vec3 osNormal;
out vec3 osTangent;
out vec2 osTexCoord;
out vec4 osShadowCoords[3];
out vec3 osWorldPos;    // L2.4: CSM computes shadow coords in the frag
out vec3 osWorldNormal; // L2.4/L1.4b: world-space normal offset


void main(void)
{
    // instance transform: R_y(rot) * (v * scale) + ipos (object-local)
    float cr = iRotMisc.x;
    float sr = iRotMisc.y;
    mat3 ry = mat3( cr, 0.0, -sr,
                    0.0, 1.0, 0.0,
                    sr, 0.0, cr );
    vec4 lv = vec4( ry * ( vxVertex.xyz * iPosScale.w ) + iPosScale.xyz,
                    vxVertex.w );
    vec3 ln = ry * vxNormal;

    gl_Position = pMatrix * mvMatrix * lv;

    osTexCoord = vxTexCoord;
    osVertex = vec3( mvMatrix * lv );
    osNormal = normalize( nMatrix * ln );
    osTangent = normalize( nMatrix * vxTangent );

    // base colour, dimmed/brightened per instance
    osColour = vec4( vec3( iRotMisc.z ), 1.0 );

    // Shadow coordinate calculation for up to 3 lights

    vec4 worldPos = mMatrix * lv;
    // L1.4b: normal-offset bias — push the lookup position ~2 texels
    // along the world normal; kills acne + peter-panning without a
    // large depth bias. Per-vertex is exact on flat surfaces and
    // negligible on dense meshes.
    vec3 worldNormal = normalize(mat3(mMatrix) * ln);
    osWorldNormal = worldNormal;
    osWorldPos = worldPos.xyz;
    vec3 offsetWorldPos = worldPos.xyz + worldNormal * 0.02;
    for (int i = 0; i < 3; ++i) {
        osShadowCoords[i] = sMatrices[i] * vec4(offsetWorldPos, 1.0);
    }
}
