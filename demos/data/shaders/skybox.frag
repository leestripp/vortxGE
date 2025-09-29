#version 330 core

in vec3 vs_viewDir;

out vec4 FragColor;

uniform sampler2D tex_skybox; // Equirectangular HDRI

void main()
{
    vec3 normalizedViewDir = normalize(vs_viewDir);
    
    // Convert cartesian to spherical coordinates for equirectangular map
    // Azimuthal angle: phi = atan2(z, x) - ranges from -PI to PI
    // Polar angle: theta = acos(y) - ranges from 0 to PI
    float phi = atan(normalizedViewDir.z, normalizedViewDir.x);
    float theta = acos(normalizedViewDir.y);

    // Map to UV coordinates (0 to 1)
    // U: (phi + PI) / (2 * PI) - maps [-PI, PI] to [0, 1]
    // V: theta / PI - maps [0, PI] to [0, 1]
    float u = (phi + 3.141592653589793) / (2.0 * 3.141592653589793);
    float v = theta / 3.141592653589793;
    
    FragColor = texture(tex_skybox, vec2(u, v));
}
