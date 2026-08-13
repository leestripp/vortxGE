#version 430

// textures
uniform sampler2DShadow tex_shadowmaps[3]; // Changed to array
uniform int use_shadowmap[3];        // Changed to array

// L2.4 CSM (bound when a light slot has use_shadowmap == 2)
uniform sampler2DShadow tex_csm;
uniform mat4 csmMatrices[4];
uniform float csmSplits[4];
uniform int csmCount;

// L3.2 point shadows (bound when a light slot has use_shadowmap == 3)
// NOTE: one point shadow caster at a time (shared cube unit 7)
uniform samplerCubeShadow tex_pointshadow;
uniform vec3 pointWorldPos[3];
uniform float pointNear[3];
uniform float pointFar[3];

// Material
uniform vec4 vxMatAlbedo;
uniform float vxMatRoughness;
uniform float vxMatMetallic;

// L4.1 PCSS: raw (non-comparison) views of tex_shadowmaps for the
// blocker search + per-slot max penumbra (uv units); High preset only
uniform sampler2D tex_shadowdepth[3];
uniform float shadowSoftness[3];
uniform int pcssEnabled;

// L4.2 contact shadows: L4.1b prepass depth + main-camera projection
// (pMatrix is shared with the vertex stage — same program uniform)
uniform sampler2D tex_prepassDepth;
uniform int contactEnabled;
uniform float contactNear;
uniform float contactFar;
uniform int prepassFlipY;
uniform mat4 pMatrix;

// Lighting
uniform vec3 vxLightPos[64];
uniform vec4 vxLightDiffuse[64];
uniform float vxLightIntensity[64];
uniform int vxLightType[64];
uniform vec3 vxLightDir[64];
uniform float vxLightCutoff[64];
uniform float vxLightOuterCutoff[64];
uniform int vxLightCount;
uniform int vxDebugView; // D1.2: 0=off 1=lightcount 2=shadow 3=normals 4=depth

// from vertex shader
in vec3 osVertex;
in vec3 osNormal;
in vec3 osTangent;
in vec2 osTexCoord;
in vec4 osShadowCoords[3]; // Changed to array
in vec3 osWorldPos;    // L2.4: CSM computes shadow coords in the frag
in vec3 osWorldNormal; // L2.4/L1.4b: world-space normal offset

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

// L1.4: 16-tap rotated Poisson-disk PCF. L1.3: the map is now a
// comparison sampler (sampler2DShadow, LINEAR + COMPARE_REF_TO_TEXTURE)
// so each tap returns a hardware bilinear depth comparison — the
// effective kernel is 16 x 4 taps.
float calculateShadow(vec4 shadowCoord, sampler2DShadow shadowMap, int slot) {
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

	// L4.1 PCSS (L4.3: High preset only): blocker search over the raw
	// depth view, then penumbra width from the receiver-blocker depth
	// separation — shadows sharpen near occluder contact, soften with
	// distance. shadowSoftness = max penumbra half-width (uv units).
	if (pcssEnabled == 1) {
		float zr = projCoords.z - bias;
		float maxRadius = max(shadowSoftness[slot] / texelSize.x, 1.0); // texels
		float blockerSum = 0.0, blockerCount = 0.0;
		for (int i = 0; i < 16; ++i) {
			vec2 offset = rot * poissonDisk[i] * texelSize * maxRadius;
			float d = texture(tex_shadowdepth[slot], projCoords.xy + offset).r;
			if (d < zr) { blockerSum += d; blockerCount += 1.0; }
		}
		if (blockerCount < 0.5) return 1.0; // no occluder in range
		float zb = blockerSum / blockerCount;
		float radius = clamp(maxRadius * (zr - zb) / max(zb, 0.02), 1.0, maxRadius);
		float shadow = 0.0;
		for (int i = 0; i < 16; ++i) {
			vec2 offset = rot * poissonDisk[i] * texelSize * radius;
			shadow += texture(shadowMap, vec3(projCoords.xy + offset, zr));
		}
		return shadow / 16.0;
	}

	float shadow = 0.0;
	for (int i = 0; i < 16; ++i) {
		vec2 offset = rot * poissonDisk[i] * texelSize * 2.5;
		// comparison tap: returns fraction lit (bilinear = 4-tap PCF)
		shadow += texture(shadowMap, vec3(projCoords.xy + offset, projCoords.z - bias));
	}
	return shadow / 16.0;
}

// L2.4: CSM sampling. The atlas holds up to 4 cascades in 2x2 quadrants;
// cascade i occupies texture-space offset ((i&1)*0.5, (i>>1)*0.5).
// csmMatrices[i] uses the same backend V-mirror convention as sMatrices.
float sampleCsmCascade(int ci, vec3 offsetWorldPos, vec2 texelSize) {
	vec4 sc = csmMatrices[ci] * vec4(offsetWorldPos, 1.0);
	vec3 projCoords = sc.xyz / sc.w;

	if (any(lessThan(projCoords.xyz, vec3(0.0))) || any(greaterThan(projCoords.xyz, vec3(1.0)))) {
		return 1.0;
	}

	// remap cascade-local [0,1] uv into its atlas quadrant
	vec2 atlasUv = projCoords.xy * 0.5 + vec2(float(ci & 1), float(ci >> 1)) * 0.5;

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
	float rnd = fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453);
	float cs = cos(rnd * 6.2831853), sn = sin(rnd * 6.2831853);
	mat2 rot = mat2(cs, -sn, sn, cs);

	float shadow = 0.0;
	for (int i = 0; i < 16; ++i) {
		vec2 offset = rot * poissonDisk[i] * texelSize * 2.5;
		shadow += texture(tex_csm, vec3(atlasUv + offset, projCoords.z - bias));
	}
	return shadow / 16.0;
}

// Cascade select by view-space depth + 10% blend band into the next
// cascade to hide the split seam.
float calculateShadowCsm() {
	float viewDepth = -osVertex.z;
	int ci = csmCount - 1;
	for (int k = 0; k < csmCount; ++k) {
		if (viewDepth <= csmSplits[k]) { ci = k; break; }
	}

	vec3 offsetWorldPos = osWorldPos + normalize(osWorldNormal) * 0.02;
	// atlas texel is half a cascade texel; x2 keeps the kernel width in
	// cascade texels (same footprint as the single-map PCF)
	vec2 texelSize = 2.0 / vec2(textureSize(tex_csm, 0));

	float shadow = sampleCsmCascade(ci, offsetWorldPos, texelSize);

	if (ci < csmCount - 1) {
		float splitNear = (ci == 0) ? 0.0 : csmSplits[ci - 1];
		float band = 0.1 * max(csmSplits[ci] - splitNear, 0.0001);
		float t = clamp((viewDepth - (csmSplits[ci] - band)) / band, 0.0, 1.0);
		if (t > 0.0) {
			shadow = mix(shadow, sampleCsmCascade(ci + 1, offsetWorldPos, texelSize), t);
		}
	}
	return shadow;
}

// L3.2: point shadow via cube map. The cube stores GL-convention
// perspective depth (90 deg faces, pointNear/pointFar); the reference
// depth for a fragment is reconstructed from its major-axis distance
// through the same projection, so HW comparison works unchanged.
// Normal-offset (L1.4b convention) + slope-scaled bias: overhead point
// lights see vertical faces at grazing incidence — without the offset
// they self-shadow across their whole area.
float calculatePointShadow(int slot) {
	vec3 N = normalize(osWorldNormal);
	vec3 toLight = pointWorldPos[slot] - osWorldPos;
	float NdotL = max(dot(N, normalize(toLight)), 0.0);
	vec3 d = (osWorldPos + N * 0.02) - pointWorldPos[slot];
	float za = max(abs(d.x), max(abs(d.y), abs(d.z))); // major-axis dist
	float n = pointNear[slot], f = pointFar[slot];
	// GL perspective: z_ndc = (f+n)/(f-n) - 2fn/((f-n)*za); [0,1] via bias
	float zndc = (f + n) / (f - n) - (2.0 * f * n) / ((f - n) * za);
	float ref = 0.5 * zndc + 0.5;
	float bias = mix(0.004, 0.0005, NdotL); // grazing -> more bias
	return texture(tex_pointshadow, vec4(d, ref - bias));
}



// L4.2: screen-space contact shadows — short view-space ray march
// from the fragment toward the light against the L4.1b prepass depth.
// Catches fine contact detail the shadow maps miss. High preset only.
float contactShadow(vec3 Lview) {
	if (contactEnabled != 1) return 1.0;
	const int STEPS = 12;
	const float rayLen = 0.5;   // metres
	const float thickness = 0.02;
	float hit = 0.0;
	for (int i = 1; i <= STEPS; ++i) {
		float t = float(i) / float(STEPS);
		vec3 sp = osVertex + Lview * (rayLen * t);
		vec4 clip = pMatrix * vec4(sp, 1.0);
		if (clip.w <= 0.0) continue;
		vec2 uv = clip.xy / clip.w * 0.5 + 0.5;
		if (prepassFlipY == 1) uv.y = 1.0 - uv.y; // VK offscreen V-mirror
		if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0)))) continue;
		float z = texture(tex_prepassDepth, uv).r;
		float sceneDist = contactNear * contactFar / (contactFar - z * (contactFar - contactNear));
		float rayDist = -sp.z;
		// prepass surface closer to the camera than the ray point = occluder
		if (sceneDist < rayDist - thickness) {
			hit = max(hit, 1.0 - t); // earlier hits along the ray darken more
		}
	}
	return 1.0 - hit * 0.7; // contact shadows soften, never fully blacken
}

void main(void) {
	// PBR properties from material uniforms
	vec3 albedo = vxMatAlbedo.rgb;
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
		if (i < 3 && use_shadowmap[i] == 1) {
			light_shadow = calculateShadow(osShadowCoords[i], tex_shadowmaps[i], i);
		}
		else if (i < 3 && use_shadowmap[i] == 2) {
			light_shadow = calculateShadowCsm();
		}
		else if (i < 3 && use_shadowmap[i] == 3) {
			light_shadow = calculatePointShadow(i);
		}
		
		light_shadow *= contactShadow(L); // L4.2: fine contact detail
		dbgShadow *= light_shadow; // D1.2
		Lo += (kD * albedo / PI + specular) * radiance * NdotL * light_shadow;
	}

	// Ambient lighting
	vec3 ambient = vec3(0.03) * albedo * ao;
	vec3 color = ambient + Lo;

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
