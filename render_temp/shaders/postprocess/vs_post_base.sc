$input a_position, a_texcoord0
$output v_texcoord

#include <bgfx_shader.sh>

void main() {
	gl_Position = vec4(a_position, 1.0);
	v_texcoord = a_texcoord0;
}
