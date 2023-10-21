#version 430
#pragma debug(on)
#pragma optimize(off)

// vbo data
layout (location = 0) in vec4 vxVertex;
layout (location = 4) in vec2 vxTexCoord;

uniform mat4 mvpMatrix;

out vec4 osColour;
out vec2 osTexCoord;

void main(void)
{
    gl_Position = mvpMatrix * vxVertex;

    osColour = vec4(1.0);
    osTexCoord = vxTexCoord;
}

