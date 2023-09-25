$input a_position, a_color0, a_normal, a_tangent, a_texcoord0, a_color1
$output v_normal, v_tangent, v_texcoord0, v_gpuPick, v_color0, v_worldPos

#define TEXTURE_DATA

#include <bgfx_shader.sh>
#include "../../include/model_transforms.sh"
#include "../../include/material.sh"

void main() {
	vec4 normal = a_normal * 2.0 - 1.0;
	vec4 tangent = a_tangent * 2.0 - 1.0;

	vec3 wnormal = mul(u_model[0], vec4(normal.xyz, 0.0)).xyz;
	vec3 wtangent = mul(u_model[0], vec4(tangent.xyz, 0.0)).xyz;

	v_normal = normalize(wnormal);
	v_tangent = normalize(wtangent);

	v_color0 = a_color0;
	v_gpuPick = a_color1; //  GPU PICKING

	v_texcoord0.xy = applyUVTransform(a_texcoord0.xy);
	v_texcoord0.z = a_texcoord0.z; // ATLAS ID

	TransformedData transform = applyPosTransforms(a_position, a_texcoord0.xy);
	v_worldPos = mul(u_model[0], transform.pos).xyz;
	gl_Position = transform.final;
}

