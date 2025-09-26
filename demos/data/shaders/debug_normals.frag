#version 430

// Lighting
uniform mat3 nMatrix;
uniform vec3 vxLightPos[3];
uniform vec4 vxLightDiffuse[3];
uniform vec4 vxLightSpecular[3];
uniform int vxLightType[3];
uniform int vxLightCount;

// Material
uniform vec4 vxMatAlbedo;
uniform float vxMatRoughness;
uniform float vxMatMetallic;

// from vertex shader
in vec3 osVertex;
in vec3 osNormal;
in vec2 osTexCoord;
// debug
smooth in vec4 osColour;

// output to frame buffer
out vec4 fragColour;

void main(void)
{
    fragColour = osColour;
}

