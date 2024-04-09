#ifndef NOISE_INCLUDE
#define NOISE_INCLUDE

// From "NEXT GENERATION POST PROCESSING IN CALL OF DUTY: ADVANCED WARFARE"
// http://advances.realtimerendering.com/s2014/index.html
float InterleavedGradientNoise(float2 uv) {
	const float3 magic = float3(0.06711056f, 0.00583715f, 52.9829189f);
	return frac(magic.z * frac(dot(uv, magic.xy)));
}

float InterleavedGradientNoise(float2 uv, float offset) {
	uv += offset * (float2(47, 17) * 0.695f);
	const float3 magic = float3(0.06711056f, 0.00583715f, 52.9829189f);
	return frac(magic.z * frac(dot(uv, magic.xy)));
}

#endif
