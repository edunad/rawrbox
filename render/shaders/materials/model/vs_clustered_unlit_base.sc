$input a_position, a_color0, a_texcoord0, a_color1
$output v_texcoord0, v_gpuPick, v_color0, v_worldPos

#define TEXTURE_DATA

#include <bgfx_shader.sh>
#include "../../include/model_transforms.sh"
#include "../../include/material.sh"

void main() {
    v_color0 = a_color0;
	v_gpuPick = a_color1; //  GPU PICKING

    v_texcoord0.xy = applyUVTransform(a_texcoord0.xy);
	v_texcoord0.z = a_texcoord0.z; // ATLAS ID

    TransformedData transform = applyPosTransforms(a_position, a_texcoord0.xy);
    v_worldPos = mul(u_model[0], transform.pos).xyz;
    gl_Position = transform.final;
}
