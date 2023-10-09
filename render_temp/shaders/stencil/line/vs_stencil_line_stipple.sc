$input a_position, a_color0, a_texcoord0
$output v_color0, v_stipple

#include <bgfx_shader.sh>

void main() {
    v_color0 = a_color0;
	v_stipple = a_texcoord0.x;

	gl_Position = vec4(a_position, 1.0);
}
