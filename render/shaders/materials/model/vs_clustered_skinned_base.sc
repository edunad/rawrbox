$input a_position, a_color0, a_normal, a_tangent, a_texcoord0, a_indices, a_weight
$output v_normal, v_tangent, v_texcoord0, v_color0, v_worldPos

#define TEXTURE_DATA

#include <bgfx_shader.sh>
#include "../../include/model_transforms.sh"
#include "../../include/model_skinned.sh"
#include "../../include/material.sh"

void main() {
	vec4 normal = a_normal * 2.0 - 1.0;
	vec4 tangent = a_tangent * 2.0 - 1.0;

	vec3 wnormal = mul(u_model[0], vec4(normal.xyz, 0.0)).xyz;
	vec3 wtangent = mul(u_model[0], vec4(tangent.xyz, 0.0)).xyz;

	v_normal = normalize(mul(u_view, vec4(wnormal, 0.0) ).xyz);
	v_tangent = normalize(mul(u_view, vec4(wtangent, 0.0) ).xyz);

    v_color0 = a_color0;
	v_texcoord0 = applyUVTransform(a_texcoord0.xyz);

	vec4 pos = boneTransform(a_indices, a_weight, a_position);
    TransformedData transform = applyPosTransforms(pos, a_texcoord0.xy);

    v_worldPos = mul(u_model[0], transform.pos).xyz;
	gl_Position = transform.final;
}
