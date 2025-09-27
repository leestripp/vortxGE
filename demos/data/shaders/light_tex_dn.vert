#version 430

// vbo data
layout (location = 0) in vec4 vxVertex;
layout (location = 1) in vec4 vxColour;
layout (location = 2) in vec3 vxNormal;
layout (location = 3) in vec3 vxTangent;
layout (location = 4) in vec2 vxTexCoord;

// Uniforms
uniform mat4 pMatrix; // Camera Projection
uniform mat4 mvMatrix; // Model View
uniform mat4 mMatrix; // Model Matrix (World Space Position calculation)
uniform mat3 nMatrix; // Normal Matrix
// Lighting.
// CHANGE: Array of Light Space Matrices (Bias * Proj * View)
uniform mat4 sMatrices[3];

// outputs
out vec3 osVertex;
out vec4 osColour;
out vec3 osNormal;
out vec3 osTangent;
out vec2 osTexCoord;
// CHANGE: Array of Shadow Coordinates (one for each potential light)
out vec4 osShadowCoords[3];

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
	// Calculate all 3 shadow coordinates
	for (int i = 0; i < 3; ++i) {
		osShadowCoords[i] = sMatrices[i] * worldPos;
	}
}
