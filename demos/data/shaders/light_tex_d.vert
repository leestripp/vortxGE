#version 430
#pragma debug(on)
#pragma optimize(off)

// vbo data
layout (location = 0) in vec4 vxVertex;
layout (location = 1) in vec4 vxColour;
layout (location = 2) in vec3 vxNormal;
layout (location = 3) in vec3 vxTangent;
layout (location = 4) in vec2 vxTexCoord;

// Uniforms
uniform mat4 pMatrix;
uniform mat4 mvMatrix;
uniform mat4 mMatrix; // ADD: Model Matrix
uniform mat4 sMatrix; // This will now be LightSpaceMatrix (Bias * Proj * View)
// Normal Matrix
uniform mat3 nMatrix;

// outputs
out vec3 osVertex;
out vec4 osColour;
out vec3 osNormal;
out vec3 osTangent;
out vec2 osTexCoord;
out vec4 osShadowCoord;


void main(void)
{
    gl_Position = pMatrix * mvMatrix * vxVertex;

    osTexCoord = vxTexCoord;
    osVertex = vec3( mvMatrix * vxVertex );
    osNormal = normalize( nMatrix * vxNormal );
    osTangent = normalize( nMatrix * vxTangent );

    // base colour
    osColour = vec4( 1.0 );

    // Shadow coordinate
    // CHANGE: Calculate from world space position
    vec4 worldPos = mMatrix * vxVertex;
    osShadowCoord = sMatrix * worldPos;
}
