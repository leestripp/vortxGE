#version 430

// textures
uniform sampler2D tex_diffuse;   // Albedo
uniform sampler2D tex_normal;    // Normal map
uniform sampler2D tex_specular;  // R: Metallic, G: Roughness, B: AO
uniform sampler2D tex_shadowmap;
uniform int use_shadowmap;

// Lighting
uniform vec3 vxLightPos[3];
uniform vec4 vxLightDiffuse[3];
uniform float vxLightIntensity[3];
uniform int vxLightType[3];
uniform vec3 vxLightDir[3];
uniform float vxLightCutoff[3];
uniform float vxLightOuterCutoff[3];
uniform int vxLightCount;

// from vertex shader
in vec3 osVertex;
in vec3 osNormal;
in vec3 osTangent;
in vec2 osTexCoord;
in vec4 osShadowCoord;

// output to frame buffer
out vec4 fragColour;

const float PI = 3.14159265359;

// PBR Functions /////////////////////////////////////////////////////////////

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return num / max(denom, 0.000001);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

//////////////////////////////////////////////////////////////////////////////

float calculateShadow(vec4 shadowCoord) {
    vec3 projCoords = shadowCoord.xyz / shadowCoord.w;

    if (any(lessThan(projCoords.xyz, vec3(0.0))) || any(greaterThan(projCoords.xyz, vec3(1.0)))) {
        return 1.0; // Not shadowed
    }

    float closestDepth = texture(tex_shadowmap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = 0.0001;
    float shadow = currentDepth - bias > closestDepth ? 0.3 : 1.0;
    
    return shadow;
}

void main(void) {
    // PBR properties from textures
    vec3 albedo     = texture(tex_diffuse, osTexCoord).rgb;
    vec3 normal_map = texture(tex_normal, osTexCoord).rgb * 2.0 - 1.0;
    vec4 mrao       = texture(tex_specular, osTexCoord);
    float metallic  = mrao.r;
    float roughness = mrao.g;
    float ao        = mrao.b;

    // Vectors
    vec3 N_vs = normalize(osNormal);
    vec3 T_vs = normalize(osTangent);
    T_vs = normalize(T_vs - dot(T_vs, N_vs) * N_vs); // Gram-Schmidt
    vec3 B_vs = cross(N_vs, T_vs);
    mat3 TBN = mat3(T_vs, B_vs, N_vs);

    vec3 N = normalize(TBN * normal_map);
    vec3 V = normalize(-osVertex);

    // Reflectivity at normal incidence (F0)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // Radiance accumulator
    vec3 Lo = vec3(0.0);

    // Shadow calculation
    float shadow = 1.0;
    if (use_shadowmap == 1) {
        shadow = calculateShadow(osShadowCoord);
    }

    // Loop over lights
    for (int i = 0; i < vxLightCount; ++i) {
        vec3 L = normalize(vxLightPos[i] - osVertex);
        vec3 H = normalize(V + L);
        float distance = length(vxLightPos[i] - osVertex);
        
        // Attenuation
        float attenuation = 1.0 / (distance * distance);
        
        if (vxLightType[i] == 2) { // LSLT_SPOT
            vec3 lightDir = normalize(vxLightDir[i]);
            float theta = dot(-L, lightDir);
            float spotEffect = smoothstep(vxLightOuterCutoff[i], vxLightCutoff[i], theta);
            attenuation *= spotEffect;
        }

        vec3 radiance = vxLightDiffuse[i].rgb * vxLightIntensity[i] * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
        vec3 specular     = numerator / denominator;
            
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	  
        
        float NdotL = max(dot(N, L), 0.0);        
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // Ambient lighting
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + (Lo * shadow);
    
    fragColour = vec4(color, 1.0);
}
