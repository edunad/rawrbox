$input a_position, a_normal, a_tangent, a_bitangent, a_color0, a_texcoord0
$output v_color0, v_texcoord0, v_texcoord1, v_normal, v_tangent, v_bitangent

#include <bgfx_shader.sh>
#include <../include/model_transforms.sh>

void main() {
	vec3 pos = a_position.xyz;

	vec3 normal = a_normal * 2.0 - 1.0;
	vec3 tangent = a_tangent * 2.0 - 1.0;
	vec3 bitagent = a_bitangent * 2.0 - 1.0;

	v_normal = mul(u_model[0], vec4(normal.xyz, 0.0)).xyz;
	v_tangent = mul(u_model[0], vec4(tangent.xyz, 0.0)).xyz;
	v_bitangent = mul(u_model[0], vec4(bitagent.xyz, 0.0)).xyz;

    v_color0 = a_color0;
	v_texcoord0 = a_texcoord0.xy;
	v_texcoord1 = vec4(mul(u_model[0], vec4(pos, 1.0)).xyz, 1.0); // Pass through world space position

    gl_Position = mul(u_modelViewProj, vec4(pos, 1.0)); //applyPosTransforms(a_position, v_texcoord0);
}
