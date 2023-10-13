#version 430
#pragma debug(on)
#pragma optimize(off)

// Lighting
uniform mat3 nMatrix;
uniform vec3 vxLightPos[3];
uniform vec4 vxLightDiffuse[3];
uniform vec4 vxLightSpecular[3];
uniform int vxLightType[3];
uniform int vxLightCount;

// Material
uniform vec4 vxMatAmbiant;
uniform vec4 vxMatDiffuse;
uniform vec4 vxMatSpecular;
uniform float vxMatShininess;
uniform float vxMatGlow;

// from vertex shader
smooth in vec3 osVertex;
smooth in vec3 osNormal;
smooth in vec2 osTexCoord;
// debug
smooth in vec4 osColour;

// output to frame buffer
out vec4 fragColour;

void main(void)
{
    fragColour = osColour;
}

