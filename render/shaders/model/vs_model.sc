$input a_position, a_normal, a_color0, a_texcoord0
$output v_color0, v_texcoord0, v_fragPos0, v_fragPos1, v_normal

#include <bgfx_shader.sh>
#include <../include/model_psx.sh>

void main() {
    vec4 translated = mul(u_modelViewProj, vec4(a_position, 1.0));
	gl_Position = psx_snap(translated, u_viewRect.zw / 2.);

	v_fragPos0 = mul(u_model[0], vec4(a_position, 1.0) ).xyz;
	v_fragPos1 = a_position;

    v_color0 = a_color0;
	v_texcoord0 = a_texcoord0;

	vec4 normal = a_normal * 2.0 - 1.0;
	vec3 wnormal = mul(u_model[0], vec4(normal.xyz, 0.0) ).xyz;

	v_normal = normalize(wnormal);
}
