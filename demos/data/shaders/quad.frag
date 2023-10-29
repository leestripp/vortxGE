#version 430
#pragma debug(on)
#pragma optimize(off)

uniform sampler2D tex_diffuse;

in vec4 osColour;
in vec2 osTexCoord;

// output to frame buffer
out vec4 fragColour;

// openGL's Linear RGB to sRGB
vec3 LinearToSRGB( vec3 rgb )
{
  return mix(1.055 * pow(rgb, vec3(1.0 / 2.4)) - 0.055,
        rgb * 12.92, lessThanEqual( rgb, vec3(0.0031308)) );
}

void main(void)
{
    vec4 c = texture( tex_diffuse, osTexCoord );
    fragColour = vec4( LinearToSRGB(c.rgb), c.a );
}

