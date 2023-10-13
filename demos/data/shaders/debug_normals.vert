#version 430
#pragma debug(on)
#pragma optimize(off)

// Object data
in vec4 vxVertex;
in vec3 vxNormal;
in vec3 vxTangent;
in vec2 vxTexCoord;

// Uniforms
uniform mat4 pMatrix;
uniform mat4 mvMatrix;
// Normal Matrix
uniform mat3 nMatrix;

// outputs
smooth out vec3 osVertex;
smooth out vec3 osNormal;
smooth out vec2 osTexCoord;
// debug
smooth out vec4 osColour;

void main(void)
{
    gl_Position = pMatrix * mvMatrix * vxVertex;

    // Interp values
    osTexCoord = vxTexCoord;
    osNormal = vxNormal;

    // debug
    osColour = vec4( abs( vxNormal ), 1.0 );
}

