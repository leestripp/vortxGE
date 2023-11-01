#version 430
#pragma debug(on)
#pragma optimize(off)

// textures
uniform sampler2D tex_diffuse;

// Lighting
uniform mat3 nMatrix;
uniform vec3 vxLightPos[3];
uniform vec4 vxLightDiffuse[3];
uniform vec4 vxLightSpecular[3];
uniform int vxLightType[3];
uniform int vxLightCount;

// Material
uniform vec4 vxMatAmbiant;
uniform vec4 vxMatDiffuse;
uniform vec4 vxMatSpecular;
uniform float vxMatShininess;
uniform float vxMatGlow;

// from vertex shader
in vec3 osVertex;
in vec4 osColour;
in vec3 osNormal;
in vec3 osTangent;
in vec2 osTexCoord;

// output to frame buffer
out vec4 fragColour;

void main(void)
{
    vec3 normal = normalize( osNormal );
    vec3 viewVec = normalize( -osVertex );

    // Lighting - lights are in camera space
    vec3 lightVec;
    vec3 reflectVec;
    float spec;
    vec3 ambientContrib = vxMatAmbiant.rgb;
    vec3 specContrib = vec3(0.0);
    vec3 diffContrib = vec3(0.0);
    vec4 texColour = texture( tex_diffuse, osTexCoord );

    int i;
    for( i=0; i<vxLightCount; i++ )
    {
        lightVec = normalize( vxLightPos[i] - osVertex );
        reflectVec = reflect( -lightVec, normal );
        // Spec factor
        spec = max( dot( reflectVec, viewVec ), 0.0 );
        spec = pow( spec, vxMatShininess );
        // Contribs
        specContrib += vxMatSpecular.rgb * vxLightSpecular[i].rgb * spec;
        diffContrib += vxMatDiffuse.rgb * vxLightDiffuse[0].rgb * max( dot( lightVec, normal ), 0.0 );
    }

    vec3 lightContrib = ambientContrib + diffContrib + specContrib;

    fragColour = vec4( (texColour.rgb * lightContrib) + specContrib, 1.0 );
    // fragColour = vec4( texColour.rgb * lightContrib, 1.0 );
}

