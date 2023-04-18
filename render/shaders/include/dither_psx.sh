#ifndef INCLUDED_DITHER_PSX
#define INCLUDED_DITHER_PSX

vec3 GetDither(vec2 pos, vec3 c, float intensity) {
	mat4 DITHER_THRESHOLDS;

	DITHER_THRESHOLDS[0][0] = -4.0;
	DITHER_THRESHOLDS[0][1] = 0.0;
	DITHER_THRESHOLDS[0][2] = -3.0;
	DITHER_THRESHOLDS[0][3] = 1.0;

	DITHER_THRESHOLDS[1][0] = 2.0;
	DITHER_THRESHOLDS[1][1] = -2.0;
	DITHER_THRESHOLDS[1][2] = 3.0;
	DITHER_THRESHOLDS[1][3] = -1.0;

	DITHER_THRESHOLDS[2][0] = -3.0;
	DITHER_THRESHOLDS[2][1] = 1.0;
	DITHER_THRESHOLDS[2][2] = -4.0;
	DITHER_THRESHOLDS[2][3] = 0.0;

	DITHER_THRESHOLDS[3][0] = 3.0;
	DITHER_THRESHOLDS[3][1] = -1.0;
	DITHER_THRESHOLDS[3][2] = 2.0;
	DITHER_THRESHOLDS[3][3] = -2.0;

	int DITHER_COLORS = 256;
	int index = (int(pos.x) & 3) * 4 + (int(pos.y) & 3);

	c.rgb = clamp(c.rgb * (float(DITHER_COLORS) - 1.0) + DITHER_THRESHOLDS[index].xyz * (intensity * 100.0), vec3(0, 0, 0), vec3(float(DITHER_COLORS) - 1.0, float(DITHER_COLORS) - 1.0, float(DITHER_COLORS) - 1.0));
	c /= float(DITHER_COLORS);

	return c;
}
#endif
