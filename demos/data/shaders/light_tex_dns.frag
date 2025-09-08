#version 430
#pragma debug(on)
#pragma optimize(off)

// textures
uniform sampler2D tex_diffuse;
uniform sampler2D tex_normal;
uniform sampler2D tex_specular;
uniform sampler2D tex_shadowmap;
uniform int use_shadowmap;

// Lighting
uniform mat3 nMatrix;
uniform vec3 vxLightPos[3];
uniform vec4 vxLightDiffuse[3];
uniform vec4 vxLightSpecular[3];
uniform int vxLightType[3];
uniform vec3 vxLightDir[3];
uniform float vxLightCutoff[3];
uniform float vxLightOuterCutoff[3];
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
in vec4 osShadowCoord;

// output to frame buffer
out vec4 fragColour;


float calculateShadow(vec4 shadowCoord)
{
    vec3 projCoords = shadowCoord.xyz / shadowCoord.w;
    
    // Don't shadow fragments outside of the light's frustum
    if (projCoords.z > 1.0) {
        return 1.0;
    }

    float closestDepth = texture(tex_shadowmap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    
    // With glPolygonOffset handling most of the work, we can use a much smaller bias.
    float bias = 0.0005; 
    
    float shadow = currentDepth - bias > closestDepth  ? 0.3 : 1.0;
    
    return shadow;
}

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
    vec4 texSpecular = texture( tex_specular, osTexCoord );

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

    float shadow = 1.0;
    if (use_shadowmap == 1) {
        shadow = calculateShadow(osShadowCoord);
    }

    for( i=0; i<vxLightCount; i++ )
    {
        lightVec = normalize( vxLightPos[i] - osVertex );
        lightVec_tangentspace = TBN * lightVec;

        float attenuation = 1.0;
        if (vxLightType[i] == 2) // 2 is LSLT_SPOT
        {
            vec3 lightDirection = normalize(vxLightDir[i]);
            vec3 lightDir_tangentspace = TBN * lightDirection;
            float theta = dot(lightVec_tangentspace, -lightDir_tangentspace);
            attenuation = smoothstep(vxLightOuterCutoff[i], vxLightCutoff[i], theta);
        }

        reflectVec = reflect( -lightVec_tangentspace, texNormal_tangentspace );

        // Spec factor
        spec = max( dot( reflectVec, viewVec_tangentspace ), 0.0 );
        spec = pow( spec, vxMatShininess ); // 16.0
        // Contribs
        specContrib += vxMatSpecular.rgb * vxLightSpecular[i].rgb * spec * attenuation;
        diffContrib += vxMatDiffuse.rgb * vxLightDiffuse[i].rgb * max( dot( lightVec_tangentspace, texNormal_tangentspace ), 0.0 ) * attenuation;
    }
    vec3 lightContrib = ambientContrib + (diffContrib * shadow);
    specContrib *= texSpecular.rgb * shadow;

    fragColour = vec4( (texColour.rgb * lightContrib) + specContrib, 1.0 );
}
