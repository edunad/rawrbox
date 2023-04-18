#ifndef INCLUDED_MODEL_PSX
#define INCLUDED_MODEL_PSX

// Snap vertex to achieve PSX look
vec4 psx_snap(vec4 vertex, vec2 resolution) {
	vec4 snappedPos = vertex;
	snappedPos.xyz = vertex.xyz / vertex.w;                         // convert to normalised device coordinates (NDC)
	snappedPos.xy = floor(resolution * snappedPos.xy) / resolution; // snap the vertex to the lower-resolution grid
	snappedPos.xyz *= vertex.w;                                     // convert back to projection-space

	return snappedPos;
}

#endif
