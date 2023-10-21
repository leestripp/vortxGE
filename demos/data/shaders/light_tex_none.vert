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
// Normal Matrix
uniform mat3 nMatrix;

// outputs
out vec3 osVertex;
out vec4 osColour;
out vec3 osNormal;
out vec3 osTangent;
out vec2 osTexCoord;


void main(void)
{
    gl_Position = pMatrix * mvMatrix * vxVertex;

    //
    osTexCoord = vxTexCoord;
    osVertex = vec3( mvMatrix * vxVertex );
    osNormal = normalize( nMatrix * vxNormal );
    osTangent = normalize( nMatrix * vxTangent );

    // base colour
    osColour = vec4( 1.0 );
}

