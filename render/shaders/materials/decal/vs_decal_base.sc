$input a_position, a_color0, a_texcoord0
$output v_color0, v_texcoord0, v_position

#include <bgfx_shader.sh>

void main() {
	v_texcoord0.xy = a_texcoord0.xy;
	v_texcoord0.z = 0.0;

#if BGFX_SHADER_LANGUAGE_GLSL
    v_texcoord0.y = 1.0 - v_texcoord0.y;
#endif

	v_color0 = a_color0;
    v_position = mul(u_modelViewProj, vec4(a_position, 1.0));

    gl_Position = v_position;
}
