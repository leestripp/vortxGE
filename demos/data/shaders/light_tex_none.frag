#version 430
#pragma debug(on)
#pragma optimize(off)

// textures
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
    float closestDepth = texture(tex_shadowmap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth  ? 0.3 : 1.0;
    if(projCoords.z > 1.0)
        shadow = 1.0;
    return shadow;
}

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
    int i;

    float shadow = 1.0;
    if (use_shadowmap == 1) {
        shadow = calculateShadow(osShadowCoord);
    }

    for( i=0; i<vxLightCount; i++ )
    {
        lightVec = normalize( vxLightPos[i] - osVertex );
        reflectVec = reflect( -lightVec, normal );

        float attenuation = 1.0;
        if (vxLightType[i] == 2) // 2 is LSLT_SPOT
        {
            vec3 lightDirection = normalize(vxLightDir[i]);
            float theta = dot(lightVec, -lightDirection);
            attenuation = smoothstep(vxLightOuterCutoff[i], vxLightCutoff[i], theta);
        }

        // Spec factor
        spec = max( dot( reflectVec, viewVec ), 0.0 );
        spec = pow( spec, vxMatShininess );// 16.0
        // Contribs
        specContrib += vxMatSpecular.rgb * vxLightSpecular[i].rgb * spec * attenuation;
        diffContrib += vxMatDiffuse.rgb * vxLightDiffuse[i].rgb * max( dot( lightVec, normal ), 0.0 ) * attenuation;
    }
    vec3 lightContrib = ambientContrib + (diffContrib * shadow);

    fragColour = vec4( (vxMatDiffuse.rgb * lightContrib) + (specContrib * shadow), 1.0 );
}
