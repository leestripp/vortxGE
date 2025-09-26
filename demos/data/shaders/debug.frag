#version 430

// vertex shader
 in vec4 osColour;

// output to frame buffer
out vec4 fragColour;

void main(void)
{
    /* Colour buffer */
    fragColour = osColour;
}
