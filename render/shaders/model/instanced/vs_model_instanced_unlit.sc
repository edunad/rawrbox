$input a_position, a_color0, a_texcoord0
$output v_color0, v_texcoord0

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>
#include <../../include/model_position.sh>

// For each mesh
// 1 to 4 = Matrix position
// 5 = Color
// 6 = UV override
BUFFER_RO(u_instanceData, vec4, 6);

vec4 getInstanceData(int id, int index) {
	return u_instanceData[id * 6 + index];
}

void main() {
	int id = gl_InstanceID;

	mat4 model = mtxFromCols(getInstanceData(id, 0), getInstanceData(id, 1), getInstanceData(id, 2), getInstanceData(id, 3));
	vec4 worldPos = mul(model, vec4(a_position, 1.0));
	gl_Position = mul(u_viewProj, worldPos);

	v_color0 = getInstanceData(id, 4);

	vec4 uvOffset = getInstanceData(id, 5); // x = offsetX, y = offsetY, z = totalSprites per column
	v_texcoord0 = (a_texcoord0.xy - uvOffset.xy);
}
