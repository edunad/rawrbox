$input v_color0

#include <bgfx_shader.sh>

SAMPLER2D(s_texColor, 0);

void main() {
	gl_FragColor = v_color0;
}
