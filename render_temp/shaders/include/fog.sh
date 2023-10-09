#ifndef DEFS_FOG_HEADER_GUARD
#define DEFS_FOG_HEADER_GUARD

uniform vec4 u_fogColor;

uniform vec4 u_fogSettings;
#define fogType u_fogSettings.x
#define fogEnd u_fogSettings.y
#define fogDensity u_fogSettings.z

float calcFogExp(float camToPixel) {
	float distRatio = 4.0 * camToPixel / fogEnd;
	return exp(-distRatio * fogDensity * distRatio * fogDensity);
}

float calcFogLinear(float camToPixel) {
    float fogStart = 0.0;

    float fogRange = fogEnd - fogStart;
    float fogDist = fogEnd - camToPixel;
    float fogFactor = fogDist / fogRange;

    return clamp(fogFactor, 0.0, 1.0);
}

vec4 applyFog(vec4 color, vec3 worldPos, vec4 camPos) {
    if(fogEnd <= 0.0 || fogDensity <= 0.0) return color;
	float camToPixel = length(worldPos - camPos.xyz);

    if(fogType == 0.0) {
		return mix(u_fogColor, color, calcFogLinear(camToPixel));
    } else if(fogType == 1.0) {
		return mix(u_fogColor, color, calcFogExp(camToPixel));
    }

    return color;
}

#endif // DEFS_FOG_HEADER_GUARD
