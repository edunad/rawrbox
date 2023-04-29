#ifndef INCLUDED_MODEL_SKINNED
#define INCLUDED_MODEL_SKINNED

#define NUM_BONES_PER_VERTEX 4
#define MAX_BONES            200

uniform mat4 u_bones[MAX_BONES]; // Max bones

// Snap vertex to achieve PSX look
mat4 boneTransform(ivec4 indices, vec4 weight) {
	mat4 BoneTransform = mat4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	bool skinned = false;

	for (int idx = 0; idx < NUM_BONES_PER_VERTEX; idx++) {
		if (weight[idx] > 0.0) {
			BoneTransform += u_bones[indices[idx]] * weight[idx];
			skinned = true;
		}
	}

	return skinned ? BoneTransform : mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
}

#endif
