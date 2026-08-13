#version 430

// L4.1b depth+normal prepass: standard transform, view-space normal
// out. Depth is implicit (rasterized into the depth attachment).

layout (location = 0) in vec4 vxVertex;
layout (location = 2) in vec3 vxNormal;

uniform mat4 mvpMatrix;
uniform mat3 nMatrix;

out vec3 osNormal;

void main(void)
{
	osNormal = normalize(nMatrix * vxNormal);
	gl_Position = mvpMatrix * vxVertex;
}
