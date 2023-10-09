#ifndef INCLUDED_DITHER_PSX
#define INCLUDED_DITHER_PSX

vec3 getDither(vec2 pos, vec3 c, float intensity) {
	mat4 DITHER_THRESHOLDS = mat4(
	    -4, 0, -3, 1,
	    2, -2, 3, -1,
	    -3, 1, -4, 0,
	    3, -1, 2, -2);

	int DITHER_COLORS = 256;

	int index = 0;
	#if BGFX_SHADER_LANGUAGE_GLSL
		index = 0; // TODO: ADD SUPPORT FOR NON BIT-WISE
	#else
		index = (int(pos.x) & 3) * 4 + (int(pos.y) & 3);
	#endif

	c.rgb = clamp(c.rgb * (float(DITHER_COLORS) - 1.0) + DITHER_THRESHOLDS[index].xyz * (intensity * 100.0), vec3(0, 0, 0), vec3(float(DITHER_COLORS) - 1.0, float(DITHER_COLORS) - 1.0, float(DITHER_COLORS) - 1.0));
	c /= float(DITHER_COLORS);

	return c;
}
#endif
