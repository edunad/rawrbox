$input a_position, a_color0, a_texcoord0, a_indices, a_weight
$output v_texcoord0, v_color0

#include <bgfx_shader.sh>
#include "../../include/model_transforms.sh"
#include "../../include/model_skinned.sh"

void main() {
    v_color0 = a_color0;
	v_texcoord0.xyz = a_texcoord0.xyz;

	vec4 pos = boneTransform(a_indices, a_weight, a_position);
	gl_Position = applyPosTransforms(u_viewProj, mul(u_model[0], pos).xyz, a_texcoord0.xy).final;
}
