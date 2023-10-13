#version 430
#pragma debug(on)
#pragma optimize(off)

// vortxGE
in vec4 vxVertex;
in vec2 vxTexCoord;
uniform mat4 mvpMatrix;

smooth out vec4 osColour;
smooth out vec2 osTexCoord;

void main(void)
{
    gl_Position = mvpMatrix * vxVertex;

    osColour = vec4(1.0);
    osTexCoord = vxTexCoord;
}

