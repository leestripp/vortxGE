#version 430

// vbo's
layout (location = 0) in vec4 vxVertex;
layout (location = 1) in vec4 vxColour;

uniform mat4 mvpMatrix;

// Outputs
out vec4 osColour;

void main(void)
{
    gl_Position = mvpMatrix * vxVertex;

    osColour = vxColour;
}
