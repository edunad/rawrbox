$input a_position, a_color0, a_texcoord0
$output v_texcoord0, v_color0, v_worldPos

#include <bgfx_shader.sh>
#include "../../include/model_transforms.sh"

void main() {
    v_color0 = a_color0;
	v_texcoord0 = a_texcoord0.xyz;

    TransformedData transform = applyPosTransforms(a_position, a_texcoord0.xy);
    v_worldPos = mul(u_model[0], transform.pos).xyz;
	gl_Position = transform.final;
}
