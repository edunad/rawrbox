$input a_position, a_color0, a_normal, a_tangent, a_bitangent, a_texcoord0
$output v_normal, v_tangent, v_bitangent, v_texcoord0, v_color0, v_worldpos

#include <bgfx_shader.sh>
#include <../include/model_transforms.sh>

void main() {
	vec4 normal = a_normal * 2.0 - 1.0;
	vec4 tangent = a_tangent * 2.0 - 1.0;
	vec4 bitangent = a_bitangent * 2.0 - 1.0;

	vec3 wnormal = mul(u_model[0], vec4(normal.xyz, 0.0)).xyz;
	vec3 wtangent = mul(u_model[0], vec4(tangent.xyz, 0.0)).xyz;
	vec3 wbitangent = mul(u_model[0], vec4(bitangent.xyz, 0.0)).xyz;

	v_normal = normalize(mul(u_view, vec4(wnormal, 0.0) ).xyz);
	v_tangent = normalize(mul(u_view, vec4(wtangent, 0.0) ).xyz);
	v_bitangent = normalize(mul(u_view, vec4(wbitangent, 0.0) ).xyz);

    v_color0 = a_color0;
	v_texcoord0 = a_texcoord0.xy;
    v_worldpos = mul(u_model[0], vec4(a_position, 1.0)).xyz;// TODO: FIX ME

    gl_Position = applyPosTransforms(a_position, v_texcoord0);
}
