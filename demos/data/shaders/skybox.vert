#version 330 core

// vbo data
layout (location = 0) in vec3 in_position;

uniform mat4 pMatrix;
uniform mat4 mvMatrix; // Model-View matrix, but only rotation part is used

out vec3 vs_viewDir;

void main()
{
    // Pass the vertex position (normalized direction) to the fragment shader
    vs_viewDir = in_position;
    
    // Remove translation from view matrix for skybox
    // mat3(mvMatrix) extracts the 3x3 rotation component.
    // Cast to mat4: new mat4(mat3(original_mat4)) zero-fills the translation part.
    mat4 view_no_translation = mat4(mat3(mvMatrix));
    
    // Calculate clip position
    vec4 clip_pos = pMatrix * view_no_translation * vec4(in_position, 1.0);
    
    // To ensure the skybox is always rendered at the far depth without z-fighting,
    // we set the clip-space Z component equal to the W component.
    // After perspective divide (gl_Position.z / gl_Position.w), this will result
    // in an NDC Z value of 1.0, which corresponds to the far plane.
    gl_Position = clip_pos;
    gl_Position.z = clip_pos.w; // Ensure z/w = 1.0 after division for max depth
}
