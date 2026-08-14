#version 330 core

// S-series procedural sky — fragment stage
// Renders the atmosphere gradient, sun disc + halo, moon disc and
// procedural stars from parameters computed on the CPU by lsSky.

in vec3 vs_viewDir;

out vec4 FragColor;

uniform vec3 sunDir;        // toward the sun, normalized
uniform vec3 sunColor;      // sun disc / halo color
uniform vec3 moonDir;       // toward the moon, normalized
uniform vec3 zenithColor;   // sky straight up
uniform vec3 horizonColor;  // sky at the horizon
uniform vec3 groundColor;   // haze below the horizon
uniform vec4 skyParamsA;    // x=dayFactor, y=starAmount, z=sunsetGlow, w=sunIntensity
uniform vec4 skyParamsB;    // x=moonIntensity

// Cell hash for procedural stars (stable per world direction)
float hash13(vec3 p)
{
    p = fract(p * 0.1031);
    p += dot(p, p.zyx + 31.32);
    return fract((p.x + p.y) * p.z);
}

void main()
{
    vec3 dir = normalize(vs_viewDir);
    float up = clamp(dir.y, -1.0, 1.0);

    // --- Atmosphere gradient -----------------------------------------
    vec3 sky;
    if (up >= 0.0)
        sky = mix(horizonColor, zenithColor, pow(up, 0.6));
    else
        sky = mix(horizonColor, groundColor, pow(-up, 0.5));

    // --- Warm glow around the sun near the horizon --------------------
    float sunDot = dot(dir, sunDir);
    float glow = pow(max(sunDot, 0.0), 4.0)
               * skyParamsA.z * (1.0 - abs(up));
    sky += vec3(1.0, 0.45, 0.15) * glow * 0.35;

    // --- Sun disc + halo ----------------------------------------------
    float disc = smoothstep(0.9993, 0.9997, sunDot);
    float halo = pow(max(sunDot, 0.0), 700.0) * 0.55
               + pow(max(sunDot, 0.0), 60.0)  * 0.12;
    sky += sunColor * (disc * 2.0 + halo) * skyParamsA.w;

    // --- Moon disc + faint halo ---------------------------------------
    float moonDot  = dot(dir, moonDir);
    float moonDisc = smoothstep(0.9995, 0.99985, moonDot);
    // cheap crater shading: darken one limb
    float limb = smoothstep(0.99985, 0.99995, moonDot);
    float moonHalo = pow(max(moonDot, 0.0), 900.0) * 0.35;
    vec3 moonCol = mix(vec3(0.78, 0.82, 0.92), vec3(0.55, 0.60, 0.72), limb);
    sky += moonCol * (moonDisc * 1.1) * skyParamsB.x;
    sky += vec3(0.6, 0.68, 0.85) * moonHalo * skyParamsB.x;

    // --- Stars (visible at night, above the horizon) -------------------
    float stars = skyParamsA.y;
    if (stars > 0.001 && up > -0.05)
    {
        vec3 sp   = dir * 140.0;
        vec3 cell = floor(sp);
        float rnd = hash13(cell);
        if (rnd > 0.995)
        {
            float d = length(fract(sp) - 0.5);
            float star = smoothstep(0.22, 0.02, d);
            float bright = (rnd - 0.995) / 0.005;
            sky += vec3(0.9, 0.95, 1.0) * star * (0.25 + 0.75 * bright)
                 * stars * smoothstep(-0.05, 0.10, up);
        }
    }

    FragColor = vec4(sky, 1.0);
}
