#version 430
#pragma debug(on)
#pragma optimize(off)

uniform sampler2D tex_diffuse;

smooth in vec4 osColour;
smooth in vec2 osTexCoord;

// output to frame buffer
out vec4 fragColour;

void main(void)
{
    fragColour = texture( tex_diffuse, osTexCoord );
}

