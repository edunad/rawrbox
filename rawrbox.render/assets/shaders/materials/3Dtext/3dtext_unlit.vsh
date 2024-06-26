#include "camera.fxh"
#include "unpack.fxh"
#include "vertex_bindless_uniforms.fxh"

#define TRANSFORM_BILLBOARD
#include "model_transforms.fxh"

struct VSInput {
	float3 Pos : ATTRIB0;
	float4 UV : ATTRIB1;
};

struct PSInput {
	float4 Pos : SV_POSITION;
	float2 UV : TEX_COORD;
	float4 Color : COLOR;
};

void main(in VSInput VSIn, out PSInput PSIn) {
	TransformedData transform = applyPosTransforms(VSIn.Pos, VSIn.UV.xy);
	PSIn.Pos = transform.final;

	PSIn.UV = VSIn.UV.xy;
	PSIn.Color = Unpack_RGBA8_UNORM(ColorOverride);
}
