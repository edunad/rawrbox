$input v_color0, v_stipple

#include <bgfx_shader.sh>

void main() {
	if (v_color0.a <= 0.0 || 0.125 < mod(v_stipple, 0.25) ) {
		discard;
	}

	gl_FragColor = v_color0;
}
