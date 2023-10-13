#version 430
#pragma debug(on)
#pragma optimize(off)

// vertex shader
smooth in vec4 osColour;

// output to frame buffer
out vec4 fragColour;

void main(void)
{
    /* Colour buffer */
    fragColour = osColour;
}

