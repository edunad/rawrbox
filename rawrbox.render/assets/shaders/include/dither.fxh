#ifndef INCLUDED_DITHER
#define INCLUDED_DITHER

float3 getDither(float2 pos, float3 c, float intensity) {
	int DITHER_THRESHOLDS[16] = {
	    -4, 0, -3, 1,
	    2, -2, 3, -1,
	    -3, 1, -4, 0,
	    3, -1, 2, -2};

	int DITHER_COLORS = 128;
	uint index = (uint(pos.x) & 3) * 4 + (uint(pos.y) & 3);

	c.rgb = clamp(c.rgb * (DITHER_COLORS - 1) + DITHER_THRESHOLDS[index] * (intensity * 100), min16float3(0, 0, 0), min16float3(DITHER_COLORS - 1, DITHER_COLORS - 1, DITHER_COLORS - 1));
	c /= DITHER_COLORS;

	return c;
}

float getDitherFast(float2 uv, float factor) {
	float DITHER_THRESHOLDS[16] = {
	    1.0 / 17.0, 9.0 / 17.0, 3.0 / 17.0, 11.0 / 17.0,
	    13.0 / 17.0, 5.0 / 17.0, 15.0 / 17.0, 7.0 / 17.0,
	    4.0 / 17.0, 12.0 / 17.0, 2.0 / 17.0, 10.0 / 17.0,
	    16.0 / 17.0, 8.0 / 17.0, 14.0 / 17.0, 6.0 / 17.0};

	uint index = (uint(uv.x) % 4) * 4 + uint(uv.y) % 4;
	return factor - DITHER_THRESHOLDS[index];
}

float alphaDither(float2 uv, float alpha) {
	float DITHER_THRESHOLDS[16] = {
	    1.0 / 17.0, 9.0 / 17.0, 3.0 / 17.0, 11.0 / 17.0,
	    13.0 / 17.0, 5.0 / 17.0, 15.0 / 17.0, 7.0 / 17.0,
	    4.0 / 17.0, 12.0 / 17.0, 2.0 / 17.0, 10.0 / 17.0,
	    16.0 / 17.0, 8.0 / 17.0, 14.0 / 17.0, 6.0 / 17.0};

	uint index = (uint(uv.x) % 4) * 4 + uint(uv.y) % 4;
	clip(alpha - DITHER_THRESHOLDS[index]);

	return alpha;
}

#endif
