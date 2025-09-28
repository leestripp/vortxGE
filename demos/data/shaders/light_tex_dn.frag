#version 430

// textures
uniform sampler2D tex_albedo;
uniform sampler2D tex_normal;
// CHANGE: Array of shadow map textures
uniform sampler2D tex_shadowmaps[3];
uniform int use_shadowmap[3];

// Lighting
uniform vec3 vxLightPos[3];
uniform vec4 vxLightDiffuse[3];
uniform float vxLightIntensity[3];
uniform int vxLightType[3];
uniform vec3 vxLightDir[3];
uniform float vxLightCutoff[3];
uniform float vxLightOuterCutoff[3];
uniform int vxLightCount;

// Material
uniform vec4 vxMatAlbedo;
uniform float vxMatRoughness;
uniform float vxMatMetallic;

// from vertex shader
in vec3 osVertex;
in vec3 osNormal;
in vec3 osTangent;
in vec2 osTexCoord;
// CHANGE: Array of shadow coordinates
in vec4 osShadowCoords[3];

// output to frame buffer
out vec4 fragColour;

const float PI = 3.14159265359;

// PBR Functions (omitted for brevity, they remain unchanged)
//////////////////////////////////////////////////////////////////////////////

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

// Function now accepts a specific shadow map sampler
float calculateShadow(vec4 shadowCoord, sampler2D shadowMap) {
    vec3 projCoords = shadowCoord.xyz / shadowCoord.w;
    
    // 1. Perspective Divide and Bounds Check
    // If the fragment is outside the light's view, treat it as fully lit (unshadowed).
    if (any(lessThan(projCoords.xyz, vec3(0.0))) || any(greaterThan(projCoords.xyz, vec3(1.0)))) {
        return 1.0; 
    }
    
    // 2. Setup Parameters
    float bias = 0.00001; // Add small bias to reduce shadow acne
    float shadow = 0.0;
    
    // Calculate the size of one texel
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0); 
    
    // Use a single, slightly expanded offset for a fixed soft shadow. 
    // This factor (1.5) determines the width of the penumbra.
    float softFactor = 0.25;
    vec2 offsetStep = texelSize * softFactor; 
    
    // PCF Kernel Size
    int Ks = 4;
    for (int x = -Ks; x <= Ks; ++x) {
        for (int y = -Ks; y <= Ks; ++y) {
            
            // Calculate the offset for this sample point using the softened offset step
            vec2 pcfOffset = vec2(x, y) * offsetStep;
            
            // Get the depth value from the shadow map
            float pcfDepth = texture(shadowMap, projCoords.xy + pcfOffset).r;
            
            // Compare the current fragment's depth (projCoords.z) to the recorded depth (pcfDepth)
            // If the fragment is farther (projCoords.z - bias > pcfDepth), it's in shadow (0.0).
            // Otherwise, it's lit (1.0).
            shadow += projCoords.z - bias > pcfDepth ? 0.0 : 1.0;
        }
    }
    
    // Average the samples and return the final shadow amount (0.0 to 1.0)
    float kernelArea = (2.0 * Ks + 1.0) * (2.0 * Ks + 1.0);
    return shadow / kernelArea;
}


void main(void) {
	// PBR properties
	vec3 albedo = texture(tex_albedo, osTexCoord).rgb;
	vec3 normal_map = texture(tex_normal, osTexCoord).rgb * 2.0 - 1.0;
	float metallic = vxMatMetallic;
	float roughness = clamp(1.0 - (vxMatRoughness / 256.0), 0.05, 1.0);
	float ao = 1.0;
	
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
	
	// Loop over lights
	for (int i = 0; i < vxLightCount; ++i) {
		vec3 L = normalize(vxLightPos[i] - osVertex);
		if(vxLightType[i] == 3) {
			// LSLT_DIRECT = 3
			L = normalize(vxLightDir[i]);
		}
		vec3 H = normalize(V + L);
		float distance = length(vxLightPos[i] - osVertex);
		
		// Attenuation
		// LSLT_DIRECT = 3 has no attenuation
		float attenuation = 1.0;
		if (vxLightType[i] == 1) { // LSLT_POINT
			attenuation = 1.0 / (distance * distance);
	    }
	    if (vxLightType[i] == 2) { // LSLT_SPOT
			attenuation = 1.0 / (distance * distance);
	        vec3 lightDir = normalize(vxLightDir[i]);
	        float theta = dot(-L, lightDir);
	        float spotEffect = smoothstep(vxLightOuterCutoff[i], vxLightCutoff[i], theta);
	        attenuation *= spotEffect;
	    }
		
		vec3 radiance = vxLightDiffuse[i].rgb * vxLightIntensity[i] * attenuation;

		// Cook-Torrance BRDF
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
		
		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
		vec3 specular = numerator / denominator;

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;

		float NdotL = max(dot(N, L), 0.0);
		
		// Shadow
        // Calculate the shadow factor *only* for the current light (i).
		float light_shadow = 1.0;
		if (use_shadowmap[i] == 1) {
            // Pass the correct shadow coordinate and shadow map sampler for this light
			light_shadow = calculateShadow(osShadowCoords[i], tex_shadowmaps[i]);
		}
        // Apply the shadow factor *only* to the current light's radiance contribution.
		Lo += ((kD * albedo / PI + specular) * radiance * NdotL) * light_shadow;
	}
	
	// Ambient lighting
	vec3 ambient = vec3(0.03) * albedo * ao;
    // 3. Add the ambient term (which is never shadowed) to the total shadowed radiance (Lo).
	vec3 color = ambient + Lo;

	fragColour = vec4(color, 1.0);
}
