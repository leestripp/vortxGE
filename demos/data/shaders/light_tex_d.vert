#version 430

// vbo data
layout (location = 0) in vec4 vxVertex;
layout (location = 1) in vec4 vxColour;
layout (location = 2) in vec3 vxNormal;
layout (location = 3) in vec3 vxTangent;
layout (location = 4) in vec2 vxTexCoord;

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
out vec4 osShadowCoords[3]; // Changed to array
out vec3 osWorldPos;    // L2.4: CSM computes shadow coords in the frag
out vec3 osWorldNormal; // L2.4/L1.4b: world-space normal offset


void main(void)
{
    gl_Position = pMatrix * mvMatrix * vxVertex;

    osTexCoord = vxTexCoord;
    osVertex = vec3( mvMatrix * vxVertex );
    osNormal = normalize( nMatrix * vxNormal );
    osTangent = normalize( nMatrix * vxTangent );

    // base colour
    osColour = vec4( 1.0 );

    // Shadow coordinate calculation for up to 3 lights
    vec4 worldPos = mMatrix * vxVertex;
    // L1.4b: normal-offset bias — push the lookup position ~2 texels
    // along the world normal; kills acne + peter-panning without a
    // large depth bias. Per-vertex is exact on flat surfaces and
    // negligible on dense meshes.
    vec3 worldNormal = normalize(mat3(mMatrix) * vxNormal);
    osWorldNormal = worldNormal;
    osWorldPos = worldPos.xyz;
    vec3 offsetWorldPos = worldPos.xyz + worldNormal * 0.02;
    for (int i = 0; i < 3; ++i) {
        osShadowCoords[i] = sMatrices[i] * vec4(offsetWorldPos, 1.0);
    }
}
