$input a_position, a_color0, a_normal, a_tangent, a_bitangent, a_texcoord0
$output v_normal, v_tangent, v_bitangent, v_texcoord0, v_color0, v_worldPos

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>
#include "../../include/model_transforms.sh"

// For each mesh
// 1 to 4 = Matrix position
// 5 = color
// 6 = UV override
BUFFER_RO(u_instanceData, vec4, 0);

vec4 getInstanceData(int id, int index) {
	return u_instanceData[id * 6 + index];
}

void main() {
	int id = gl_InstanceID;

	mat4 model = mtxFromCols(getInstanceData(id, 0), getInstanceData(id, 1), getInstanceData(id, 2), getInstanceData(id, 3));

	vec4 normal = a_normal * 2.0 - 1.0;
	vec4 tangent = a_tangent * 2.0 - 1.0;
	vec4 bitangent = a_bitangent * 2.0 - 1.0;

	vec3 wnormal = mul(u_model[0], vec4(normal.xyz, 0.0)).xyz;
	vec3 wtangent = mul(u_model[0], vec4(tangent.xyz, 0.0)).xyz;
	vec3 wbitangent = mul(u_model[0], vec4(bitangent.xyz, 0.0)).xyz;

	v_normal = normalize(mul(u_view, vec4(wnormal, 0.0) ).xyz);
	v_tangent = normalize(mul(u_view, vec4(wtangent, 0.0) ).xyz);
	v_bitangent = normalize(mul(u_view, vec4(wbitangent, 0.0) ).xyz);

    v_worldPos = mul(model, vec4(a_position, 1.));
	v_color0 = getInstanceData(id, 4);

	v_texcoord0.xy = a_texcoord0.xy;
	v_texcoord0.z = getInstanceData(id, 5).x;

	gl_Position = applyPosTransforms(u_viewProj, v_worldPos, a_texcoord0.xy);
}
