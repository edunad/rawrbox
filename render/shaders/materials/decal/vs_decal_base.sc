$input a_position, a_color0, a_texcoord0, a_normal
$output v_texcoord0, v_color0, v_model_0, v_model_1, v_model_2, v_model_3, v_normal, v_worldPos

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>
#include <../../include/defs.sh>

// For each mesh
// 1 to 4 = Matrix position
// 5 = color
// 6 = UV override
BUFFER_RO(u_instanceData, vec4, SAMPLE_INSTANCE_DATA);

vec4 getInstanceData(int id, int index) {
	return u_instanceData[id * 6 + index];
}

void main() {
	uint id = gl_InstanceID;

	// Not sure if this is the best way to do it, idk if many outputs affect performance
	v_model_0 = getInstanceData(id, 0);
	v_model_1 = getInstanceData(id, 1);
	v_model_2 = getInstanceData(id, 2);
	v_model_3 = getInstanceData(id, 3);

	mat4 model = mtxFromCols(v_model_0, v_model_1, v_model_2, v_model_3);

	v_color0 = a_color0 * getInstanceData(id, 4);

	v_texcoord0.xy = a_texcoord0.xy;
	v_texcoord0.z = getInstanceData(id, 5).x;

	vec4 normal = a_normal * 2.0 - 1.0;

	vec3 wnormal = mul(model, vec4(normal.xyz, 0.0)).xyz;

	v_normal = normalize(wnormal);
	v_worldPos = mul(model, vec4(a_position, 1.)).xyz;

	gl_Position = mul(u_viewProj, vec4(v_worldPos, 1.));
}


