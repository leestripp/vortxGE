#version 430

// textures
uniform sampler2D tex_albedo;
uniform sampler2D tex_shadowmaps[3]; // Changed to array
uniform int use_shadowmap[3];        // Changed to array

// Lighting
uniform vec3 vxLightPos[3];
uniform vec4 vxLightDiffuse[3];
uniform float vxLightIntensity[3];
uniform int vxLightType[3];
uniform vec3 vxLightDir[3];
uniform float vxLightCutoff[3];
uniform float vxLightOuterCutoff[3];
uniform int vxLightCount;
uniform int vxDebugView; // D1.2: 0=off 1=lightcount 2=shadow 3=normals 4=depth

// Material
uniform vec4 vxMatAlbedo;
uniform float vxMatRoughness;
uniform float vxMatMetallic;

// from vertex shader
in vec3 osVertex;
in vec3 osNormal;
in vec3 osTangent;
in vec2 osTexCoord;
in vec4 osShadowCoords[3]; // Changed to array

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

// L1.4: 16-tap rotated Poisson-disk PCF (replaces the pseudo-9x9 grid
// that sampled a sub-texel area). Comparison bias stays small; the
// polygon-offset depth bias in the shadow stage does the heavy lifting.
float calculateShadow(vec4 shadowCoord, sampler2D shadowMap) {
	vec3 projCoords = shadowCoord.xyz / shadowCoord.w;

	if (any(lessThan(projCoords.xyz, vec3(0.0))) || any(greaterThan(projCoords.xyz, vec3(1.0)))) {
		return 1.0;
	}

	const vec2 poissonDisk[16] = vec2[](
		vec2(-0.94201624, -0.39906216), vec2( 0.94558609, -0.76890725),
		vec2(-0.09418410, -0.92938870), vec2( 0.34495938,  0.29387760),
		vec2(-0.91588581,  0.45771432), vec2(-0.81544232, -0.87912464),
		vec2(-0.38277543,  0.27676845), vec2( 0.97484398,  0.75648379),
		vec2( 0.44323325, -0.97511554), vec2( 0.53742981, -0.47373420),
		vec2(-0.26496911, -0.41893023), vec2( 0.79197514,  0.19090188),
		vec2(-0.24188840,  0.99706507), vec2(-0.81409955,  0.91437590),
		vec2( 0.19984126,  0.78641367), vec2( 0.14383161, -0.14100790));

	float bias = 0.0001;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

	// per-pixel random rotation: banding -> (temporal-friendly) noise
	float rnd = fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453);
	float cs = cos(rnd * 6.2831853), sn = sin(rnd * 6.2831853);
	mat2 rot = mat2(cs, -sn, sn, cs);

	float shadow = 0.0;
	for (int i = 0; i < 16; ++i) {
		vec2 offset = rot * poissonDisk[i] * texelSize * 2.5;
		float pcfDepth = texture(shadowMap, projCoords.xy + offset).r;
		shadow += projCoords.z - bias > pcfDepth ? 0.0 : 1.0;
	}
	return shadow / 16.0;
}

void main(void) {
	// PBR properties
	vec3 albedo = texture(tex_albedo, osTexCoord).rgb;
	float metallic = vxMatMetallic;
	float roughness = clamp(1.0 - (vxMatRoughness / 256.0), 0.05, 1.0);
	float ao = 1.0;
	
	// Vectors
	vec3 N = normalize(osNormal);
	vec3 V = normalize(-osVertex);

	// Reflectivity at normal incidence (F0)
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	// Radiance accumulator
	vec3 Lo = vec3(0.0);
	float dbgShadow = 1.0; // D1.2: product of per-light shadow factors

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
		// LSLT_DIRECT has no attenuation
		float attenuation = 1.0;
		if (vxLightType[i] == 1) { // LSLT_POINT
			attenuation = attenuation / (distance * distance);
	    }
	    if (vxLightType[i] == 2) { // LSLT_SPOT
			attenuation = attenuation / (distance * distance);
	        vec3 lightDir = normalize(vxLightDir[i]);
	        float theta = dot(-L, lightDir);
	        float spotEffect = smoothstep(vxLightOuterCutoff[i], vxLightCutoff[i], theta);
	        attenuation *= spotEffect;
	    }
		
		vec3 radiance = vxLightDiffuse[i].rgb * vxLightIntensity[i] * attenuation;

		// Cook-Torrance BRDF
		float NDF = DistributionGGX(N, H, roughness);
		// roughness
		vec3 rld = L;
		if(vxLightType[i] == 3) {
				// LSLT_DIRECT = 3
				rld = normalize(vxLightDir[i]);
		}
		float G = GeometrySmith(N, V, rld, roughness);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
		vec3 specular = numerator / denominator;

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;

		float NdotL = max(dot(N, L), 0.0);
		
		// Shadow calculation for this light
		float light_shadow = 1.0;
		if (use_shadowmap[i] == 1) {
			light_shadow = calculateShadow(osShadowCoords[i], tex_shadowmaps[i]);
		}
		
		dbgShadow *= light_shadow; // D1.2
		Lo += (kD * albedo / PI + specular) * radiance * NdotL * light_shadow;
	}

	// Ambient lighting
	vec3 ambient = vec3(0.03) * albedo * ao;
	vec3 color = ambient + Lo; // Lo already includes shadow per-light

		// D1.2 debug views (vxDebugView; driven by VORTX_DEBUG_VIEW in
	// debug-tools builds, 0 otherwise)
	if (vxDebugView == 1) {      // lightcount heat map: blue=0 .. red=3
		float t = clamp(float(vxLightCount) / 3.0, 0.0, 1.0);
		fragColour = vec4(t, 0.0, 1.0 - t, 1.0);
		return;
	}
	if (vxDebugView == 2) {      // shadow factor: white=lit, black=shadowed
		fragColour = vec4(vec3(dbgShadow), 1.0);
		return;
	}
	if (vxDebugView == 3) {      // view-space normals as color
		fragColour = vec4(normalize(osNormal) * 0.5 + 0.5, 1.0);
		return;
	}
	if (vxDebugView == 4) {      // camera distance: near=white, 30m+=black
		float d = clamp(length(osVertex) / 30.0, 0.0, 1.0);
		fragColour = vec4(vec3(1.0 - d), 1.0);
		return;
	}

	fragColour = vec4(color, 1.0);
}
