$input a_position, a_normal, a_tangent, a_bitangent, a_color0, a_texcoord0, a_indices, a_weight
$output v_color0, v_texcoord0, v_wPos, v_normal, v_tangent, v_bitangent

#include <bgfx_shader.sh>
#include <../../../include/model_psx.sh>
#include <../../../include/model_skinned.sh>

void main() {
	vec4 world = mul(u_model[0], boneTransform(a_indices, a_weight, a_position) );

	vec4 translated = mul(u_viewProj, world);
	gl_Position = psx_snap(translated, u_viewRect.zw / 2.);

    v_color0 = a_color0;
	v_texcoord0 = a_texcoord0;

	vec3 normal = a_normal * 2.0 - 1.0;
	vec3 tangent = a_tangent * 2.0 - 1.0;
	vec3 bitagent = a_bitangent * 2.0 - 1.0;
	vec3 wnormal = mul(u_model[0], vec4(normal.xyz, 0.0) ).xyz;
	vec3 wtangent = mul(u_model[0], vec4(tangent.xyz, 0.0) ).xyz;
	vec3 wbitagent = mul(u_model[0], vec4(bitagent.xyz, 0.0) ).xyz;

	v_normal = normalize(wnormal);
	v_tangent = normalize(wtangent);
	v_bitangent = normalize(wbitagent);

	v_wPos = world.xyz;
}
