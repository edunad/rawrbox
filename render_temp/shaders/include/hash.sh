
#ifndef INCLUDED_HASH
#define INCLUDED_HASH
// https://www.pcg-random.org/
uint pcg(uint v) {
	uint state = v * 747796405u + 2891336453u;
	uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;

	return (word >> 22u) ^ word;
}

uvec2 pcg2d(uvec2 v) {
	v = v * 1664525u + 1013904223u;

	v.x += v.y * 1664525u;
	v.y += v.x * 1664525u;

	v = v ^ (v >> 16u);

	v.x += v.y * 1664525u;
	v.y += v.x * 1664525u;

	v = v ^ (v >> 16u);

	return v;
}

// http://www.jcgt.org/published/0009/03/02/
uvec3 pcg3d(uvec3 v) {

	v = v * 1664525u + 1013904223u;

	v.x += v.y * v.z;
	v.y += v.z * v.x;
	v.z += v.x * v.y;

	// v = ((v >> int((v >> 28u) + 4u)) ^ v) * 277803737u;
	v ^= v >> 16u;

	v.x += v.y * v.z;
	v.y += v.z * v.x;
	v.z += v.x * v.y;

	return v;
}

// http://www.jcgt.org/published/0009/03/02/
uvec3 pcg3d16(uvec3 v) {
	v = v * 12829u + 47989u;

	v.x += v.y * v.z;
	v.y += v.z * v.x;
	v.z += v.x * v.y;

	v.x += v.y * v.z;
	v.y += v.z * v.x;
	v.z += v.x * v.y;

	v >>= 16u;

	return v;
}

// http://www.jcgt.org/published/0009/03/02/
uvec4 pcg4d(uvec4 v) {
	v = v * 1664525u + 1013904223u;

	v.x += v.y * v.w;
	v.y += v.z * v.x;
	v.z += v.x * v.y;
	v.w += v.y * v.z;

	v ^= v >> 16u;

	v.x += v.y * v.w;
	v.y += v.z * v.x;
	v.z += v.x * v.y;
	v.w += v.y * v.z;

	return v;
}
#endif
