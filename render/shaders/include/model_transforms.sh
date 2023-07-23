

#ifndef INCLUDED_MODEL_TRANSFORMS
#define INCLUDED_MODEL_TRANSFORMS

#define VERTEX_DATA

#include "defs.sh"
#include "material.sh"

uniform vec3 u_mesh_pos;
SAMPLER2D(s_displacement, SAMPLE_MAT_DISPLACEMENT);

struct TransformedData {
    vec4 pos;
    vec4 final;
};

// Snap vertex to achieve PSX look
vec4 psx_snap(vec4 vertex, vec2 resolution) {
	vec4 snappedPos = vertex;
	snappedPos.xyz = vertex.xyz / vertex.w;                         // convert to normalised device coordinates (NDC)
	snappedPos.xy = floor(resolution * snappedPos.xy) / resolution; // snap the vertex to the lower-resolution grid
	snappedPos.xyz *= vertex.w;                                     // convert back to projection-space

	return snappedPos;
}
// ----------------------

// Snap vertex to achieve PSX look
TransformedData applyPosTransforms(mat4 proj, vec4 a_position, vec2 a_texcoord0) {
    TransformedData data;
    data.pos = a_position;

    // displacement mode
    if(displacement_power != 0.) {
	    data.pos.y += texture2DLod(s_displacement, a_texcoord0, 0).x * displacement_power;
    }
    // ----

    // Billboard mode
    // TOOD: Lock X Y Z using billboard
    if(billboard.x != 0. || billboard.y != 0. || billboard.z != 0.) {
        vec3 right = vec3(u_invView[0][0], u_invView[1][0], u_invView[2][0]);
        vec3 up = vec3(u_invView[0][1], u_invView[1][1], u_invView[2][1]);

        data.pos = vec4(u_mesh_pos.xyz + (right * (data.pos.x - u_mesh_pos.x)) + (up * (data.pos.y - u_mesh_pos.y)), 1.);
    }
    // ----

	// vertex_snap mode
    if(vertexSnap != 0.) {
        data.final = psx_snap(mul(proj, data.pos), u_viewRect.zw / vertexSnap);
    } else {
        data.final = mul(proj, data.pos);
    }
    // ----

    return data;
}

TransformedData applyPosTransforms(vec4 a_position, vec2 a_texcoord0) {
    return applyPosTransforms(u_modelViewProj, a_position, a_texcoord0);
}

TransformedData applyPosTransforms(vec3 a_position, vec2 a_texcoord0) {
    return applyPosTransforms(u_modelViewProj, vec4(a_position, 1.0), a_texcoord0);
}

TransformedData applyPosTransforms(mat4 proj, vec3 a_position, vec2 a_texcoord0) {
    return applyPosTransforms(proj, vec4(a_position, 1.0), a_texcoord0);
}
// ----------------------
#endif
