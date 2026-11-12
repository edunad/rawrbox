#ifndef INCLUDED_PARTICLE_VERTEX
#define INCLUDED_PARTICLE_VERTEX

#include "include/unpack.fxh"

#define READ_PARTICLES
#include "plugins/particle_engine/include/particles.fxh"

struct VSInput {
	uint VertexID : SV_VertexID;
};

struct PSInput {
	float4 POS : SV_POSITION;
	float2 SIZE : SIZE;
	float3x3 ROTATION : ROTATION;
	float3 UV : TEXCOORD0;
	float4 COLOR : COLOR;
};

float3x3 makeRotationMatrix(float3 rotation) {
	float cosX = cos(rotation.x);
	float sinX = sin(rotation.x);
	float cosY = cos(rotation.y);

	float sinY = sin(rotation.y);
	float cosZ = cos(rotation.z);
	float sinZ = sin(rotation.z);

	float3x3 rotX = float3x3(
	    1, 0, 0,
	    0, cosX, -sinX,
	    0, sinX, cosX);

	float3x3 rotY = float3x3(
	    cosY, 0, sinY,
	    0, 1, 0,
	    -sinY, 0, cosY);

	float3x3 rotZ = float3x3(
	    cosZ, -sinZ, 0,
	    sinZ, cosZ, 0,
	    0, 0, 1);

	// ZYX
	return mul(mul(rotZ, rotY), rotX);
}

PSInput particleVertex(uint vertexID) {
	PSInput output;
	Particle particle = GetParticle(vertexID);

	output.POS = float4(particle.position, 1.0);
	output.UV = float3(0.0, 0.0, particle.atlasIndex);
	output.SIZE = Unpack_RG16_FLOAT(particle.size);
	output.ROTATION = makeRotationMatrix(particle.rotation);
	output.COLOR = particle.color;
	return output;
}

#endif