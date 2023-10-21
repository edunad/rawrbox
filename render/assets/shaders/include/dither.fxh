#ifndef INCLUDED_DITHER_PSX
#define INCLUDED_DITHER_PSX

float3 getDither(float2 pos, float3 c, float intensity) {
	float4x4 DITHER_THRESHOLDS = float4x4(
	    -4, 0, -3, 1,
	    2, -2, 3, -1,
	    -3, 1, -4, 0,
	    3, -1, 2, -2);

	int DITHER_COLORS = 256;
	int index = (int(pos.x) & 3) * 4 + (int(pos.y) & 3);

	c.rgb = clamp(c.rgb * (float(DITHER_COLORS) - 1.0) + DITHER_THRESHOLDS[index].xyz * (intensity * 100.0), float3(0, 0, 0), float3(float(DITHER_COLORS) - 1.0, float(DITHER_COLORS) - 1.0, float(DITHER_COLORS) - 1.0));
	c /= float(DITHER_COLORS);

	return c;
}
#endif
