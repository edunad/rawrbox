$input a_position, a_color0, a_texcoord0, a_indices, a_weight
$output v_color0, v_texcoord0

#include <bgfx_shader.sh>
#include <../include/model_psx.sh>

#define NUM_BONES_PER_VERTEX 4
#define MAX_BONES 200

uniform mat4 u_bones[MAX_BONES]; // Max bones
uniform vec2 u_displayBone; // DEBUG

void main() {

	mat4 BoneTransform = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	for (int idx = 0; idx < NUM_BONES_PER_VERTEX; idx++) {
		if (a_weight[idx] > 0.0) {
			BoneTransform += u_bones[a_indices[idx]] * a_weight[idx];
		}
	}


	/*mat4 BoneTransform = u_bones[a_indices[0]] * a_weight[0];
    BoneTransform     += u_bones[a_indices[1]] * a_weight[1];
    BoneTransform     += u_bones[a_indices[2]] * a_weight[2];
    BoneTransform     += u_bones[a_indices[3]] * a_weight[3];*/

	vec4 translated = mul(u_modelViewProj, mul(BoneTransform, vec4(a_position, 1.0)));
	gl_Position = psx_snap(translated, u_viewRect.zw / 2.);

	/// TEST -------------
	bool found = false;
	vec3 texColor = vec3(0, 0, 0);
	for (int i = 0; i < NUM_BONES_PER_VERTEX; i++) {
		if (a_indices[i] == int(u_displayBone.x)) {
			if(a_weight[i] >= 0.7){
				texColor = vec3(1.0, 0.0, 0.0) * a_weight[i];
			}else if(a_weight[i] >= 0.4 && a_weight[i] <= 0.6) {
				texColor = vec3(0.0, 1.0, 0.0) * a_weight[i];
			}else if(a_weight[i] >= 0.1){
				texColor = vec3(0.0, 0.0, 1.0) * a_weight[i];
			}

			found = true;
			break;
		}
	}

	if (!found)
		v_color0 = a_color0;
	else
		v_color0 = vec4(texColor, 1.0);
	// ------------------


	v_texcoord0 = a_texcoord0;
}
