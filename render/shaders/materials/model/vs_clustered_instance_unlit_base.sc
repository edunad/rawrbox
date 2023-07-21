$input a_position, a_color0, a_texcoord0
$output v_texcoord0, v_color0

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>
#include "../../include/model_transforms.sh"

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
	mat4 model = mtxFromCols(getInstanceData(id, 0), getInstanceData(id, 1), getInstanceData(id, 2), getInstanceData(id, 3));

    v_color0 = a_color0;

	v_texcoord0.xy = a_texcoord0.xy;
	v_texcoord0.z = getInstanceData(id, 5).x;

	gl_Position = applyPosTransforms(u_viewProj, mul(model, vec4(a_position, 1.)), a_texcoord0.xy).final;
}

