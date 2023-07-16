$input a_position, a_color0, a_texcoord0
$output v_color0, v_texcoord0, v_wpos

#include <bgfx_shader.sh>

void main() {
	v_texcoord0.xy = a_texcoord0.xy;
	v_texcoord0.z = 0.0;

	v_color0 = a_color0;

    v_wpos = mul(u_model[0], vec4(a_position, 1.0)).xyz;
	gl_Position = mul(u_viewProj, vec4(v_wpos, 1.0));
}
