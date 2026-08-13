#version 430

// L4.1b: colour attachment = view-space normal (for future SSAO/SSR);
// the depth attachment (written implicitly) feeds L4.2 contact shadows.

in vec3 osNormal;
out vec4 fragColour;

void main(void)
{
	fragColour = vec4(normalize(osNormal) * 0.5 + 0.5, 1.0);
}
