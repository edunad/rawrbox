$input a_position, a_color0, a_texcoord0, a_color1, a_indices, a_weight
$output v_texcoord, v_gpuPick, v_color0, v_worldPos, v_data

#define TEXTURE_DATA

#include <bgfx_shader.sh>
#include "../../include/model_transforms.sh"
#include "../../include/model_skinned.sh"
#include "../../include/material.sh"

void main() {
    v_color0 = a_color0;
	v_gpuPick = a_color1; //  GPU PICKING

    v_texcoord.xy = applyUVTransform(a_texcoord0.xy);
	v_data.x = a_texcoord0.z; // ATLAS ID

    vec4 pos = boneTransform(a_indices, a_weight, a_position);
    TransformedData transform = applyPosTransforms(pos, a_texcoord0.xy);

    v_worldPos = mul(u_model[0], transform.pos).xyz;
    gl_Position = transform.final;
}