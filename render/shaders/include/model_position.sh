

#ifndef INCLUDED_MODEL_POSITION
#define INCLUDED_MODEL_POSITION

#define MAX_DATA 4

uniform vec4 u_data[MAX_DATA];
uniform vec3 u_mesh_pos;

SAMPLER2D(s_heightMap, 1);

// Snap vertex to achieve PSX look
vec4 psx_snap(vec4 vertex, vec2 resolution) {
	vec4 snappedPos = vertex;
	snappedPos.xyz = vertex.xyz / vertex.w;                         // convert to normalised device coordinates (NDC)
	snappedPos.xy = floor(resolution * snappedPos.xy) / resolution; // snap the vertex to the lower-resolution grid
	snappedPos.xyz *= vertex.w;                                     // convert back to projection-space

	return snappedPos;
}

// Snap vertex to achieve PSX look
vec4 getTranslatedPos(vec3 a_position, vec2 a_texcoord0) {
    vec4 pos = vec4(a_position, 1.0);

    // displacement mode
    if(u_data[2].x != 0.) {
	    pos.y += texture2DLod(s_heightMap, a_texcoord0, 0).x * u_data[2].x;
    }
    // ----

    // Billboard mode
    if(u_data[0].x != 0. || u_data[0].y != 0. || u_data[0].z != 0.){
        vec3 right = vec3(u_invView[0][0], u_invView[1][0], u_invView[2][0]);
        vec3 up = vec3(u_invView[0][1], u_invView[1][1], u_invView[2][1]);

        pos = vec4(u_mesh_pos.xyz + (right * (pos.x - u_mesh_pos.x)) + (up * (pos.y - u_mesh_pos.y)), 1.);
    }
    // ----

	// vertex_snap mode
    if(u_data[1].x != 0.) {
        return psx_snap(mul(u_modelViewProj, pos), u_viewRect.zw / u_data[1].x);
    } else {
        return mul(u_modelViewProj, pos);
    }
    // ----
}

#endif
