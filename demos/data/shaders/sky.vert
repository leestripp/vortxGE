#version 330 core

// S-series procedural sky — vertex stage
// Same trick as the skybox shader: rotation-only view matrix and
// clip-space z = w so the sky always lands on the far plane.

layout (location = 0) in vec3 in_position;

uniform mat4 pMatrix;
uniform mat4 mvMatrix; // Model-View matrix, only the rotation part is used

out vec3 vs_viewDir;

void main()
{
    vs_viewDir = in_position;

    mat4 view_no_translation = mat4(mat3(mvMatrix));

    vec4 clip_pos = pMatrix * view_no_translation * vec4(in_position, 1.0);
    gl_Position = clip_pos;
    gl_Position.z = clip_pos.w; // depth = 1.0 (far plane) after divide
}
