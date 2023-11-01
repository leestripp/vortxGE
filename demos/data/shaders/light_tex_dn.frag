#version 430
#pragma debug(on)
#pragma optimize(off)

// textures
uniform sampler2D tex_diffuse;
uniform sampler2D tex_normal;

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
    // TBN
    vec3 normal = normalize( osNormal );
    vec3 tangent = normalize( osTangent );
    vec3 binormal = cross( tangent, normal );
    mat3 TBN = transpose( mat3( tangent, binormal, normal ) );

    // View
    vec3 viewVec = normalize( -osVertex );
    vec3 viewVec_tangentspace = TBN * viewVec;

    // textures
    vec4 texColour = texture( tex_diffuse, osTexCoord );
    vec3 texNormal_tangentspace = normalize( ( 2.0 * texture( tex_normal, osTexCoord ).xyz ) -1.0 );

    // Lighting - lights are in camera space
    vec3 lightVec;
    vec3 lightVec_tangentspace;
    vec3 reflectVec;
    vec3 reflectVec_tangentspace;
    float spec;
    vec3 ambientContrib = vxMatAmbiant.rgb;
    vec3 specContrib = vec3(0.0);
    vec3 diffContrib = vec3(0.0);
    int i;

    for( i=0; i<vxLightCount; i++ )
    {
        lightVec = normalize( vxLightPos[i] - osVertex );
        lightVec_tangentspace = TBN * lightVec;

        reflectVec = reflect( -lightVec_tangentspace, texNormal_tangentspace );

        // Spec factor
        spec = max( dot( reflectVec, viewVec_tangentspace ), 0.0 );
        spec = pow( spec, vxMatShininess ); // 16.0
        // Contribs
        specContrib += vxMatSpecular.rgb * vxLightSpecular[i].rgb * spec;
        diffContrib += vxMatDiffuse.rgb * vxLightDiffuse[0].rgb * max( dot( lightVec_tangentspace, texNormal_tangentspace ), 0.0 );
    }

    vec3 lightContrib = ambientContrib + diffContrib;

    fragColour = vec4( (texColour.rgb * lightContrib) + specContrib, 1.0 );
}

