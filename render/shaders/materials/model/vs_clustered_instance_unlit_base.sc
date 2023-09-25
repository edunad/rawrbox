$input a_position, a_color0, a_texcoord0, a_color1
$output v_texcoord0, v_gpuPick, v_color0, v_worldPos

#define TEXTURE_DATA

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>
#include "../../include/model_transforms.sh"
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

	v_color0 = a_color0 * getInstanceData(id, 4);
	v_gpuPick = a_color1; //  GPU PICKING

	v_texcoord0.xy = applyUVTransform(a_texcoord0.xy);

	vec4 extra = getInstanceData(id, 5);
	v_texcoord0.z = extra.x; // Atlas
	v_gpuPick.rgb = extra.yzw; // GPU PICKING

	TransformedData transform = applyPosTransforms(u_viewProj, mul(model, vec4(a_position, 1.)), a_texcoord0.xy);
	v_worldPos = mul(model, transform.pos).xyz;
	gl_Position = transform.final;
}

