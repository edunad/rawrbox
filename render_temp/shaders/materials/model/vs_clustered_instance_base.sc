$input a_position, a_color0, a_normal, a_tangent, a_texcoord0
$output v_normal, v_tangent, v_texcoord, v_gpuPick, v_color0, v_worldPos, v_data

#define TEXTURE_DATA

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>
#include "../../include/model_transforms.sh"
#include "../../include/defs.sh"
#include "../../include/material.sh"

// For each mesh
// 1 to 4 = Matrix position
// 5 = color
// 6 = UV override
BUFFER_RO(u_instanceData, vec4, SAMPLE_INSTANCE_DATA);

vec4 getInstanceData(int id, int index) {
	return u_instanceData[id * 6 + index];
}

void main() {
	int id = gl_InstanceID;
	mat4 model = mul(u_model[0], mtxFromCols(getInstanceData(id, 0), getInstanceData(id, 1), getInstanceData(id, 2), getInstanceData(id, 3)));

	vec4 normal = a_normal * 2.0 - 1.0;
	vec4 tangent = a_tangent * 2.0 - 1.0;

	vec3 wnormal = mul(u_model[0], vec4(normal.xyz, 0.0)).xyz;
	vec3 wtangent = mul(u_model[0], vec4(tangent.xyz, 0.0)).xyz;

	v_normal = normalize(mul(u_view, vec4(wnormal, 0.0) ).xyz);
	v_tangent = normalize(mul(u_view, vec4(wtangent, 0.0) ).xyz);

	v_color0 = a_color0 * getInstanceData(id, 4);

	v_texcoord.xy = applyUVTransform(a_texcoord0.xy);

	vec4 extra = getInstanceData(id, 5);
	v_data.x = extra.x; // Atlas
	v_gpuPick.rgb = extra.yzw; // GPU PICKING

	TransformedData transform = applyPosTransforms(u_viewProj, mul(model, vec4(a_position, 1.)), a_texcoord0.xy);
	v_worldPos = mul(model, transform.pos).xyz;
	gl_Position = transform.final;
}

