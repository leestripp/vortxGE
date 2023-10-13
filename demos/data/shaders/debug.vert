#version 430
#pragma debug(on)
#pragma optimize(off)

// vortxGE
in vec4 vxVertex;
in vec4 vxColour;
uniform mat4 mvpMatrix;

// Outputs
smooth out vec4 osColour;

void main(void)
{
    gl_Position = mvpMatrix * vxVertex;

    osColour = vxColour;
}

