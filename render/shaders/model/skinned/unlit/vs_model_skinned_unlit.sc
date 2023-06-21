$input a_position, a_color0, a_texcoord0, a_indices, a_weight
$output v_color0, v_texcoord0

#include <bgfx_shader.sh>

#include <../../../include/model_skinned.sh>
#include <../../../include/model_transforms.sh>

void main() {
	gl_Position = applyPosTransforms(boneTransform(a_indices, a_weight, a_position), a_texcoord0.xy);

	v_texcoord0 = a_texcoord0.xy;
	v_color0 = a_color0;
}
