cbuffer Constants {
    float4x4 g_Model;
    float4x4 g_Proj;
    float4x4 g_View;
    float4x4 g_WorldViewProj;
};

struct VSInput {
    float3 Pos   : ATTRIB0;
    float4 UV    : ATTRIB1;
    float4 Color : ATTRIB2;
};

struct PSInput {
    float4 Pos   : SV_POSITION;
    float4 UV    : TEX_COORD;
    float4 Color : COLOR0;
};

void main(in VSInput VSIn, out PSInput PSIn) {
    PSIn.Pos   = mul(float4(VSIn.Pos, 1.0), g_WorldViewProj);
    PSIn.UV    = VSIn.UV;
    PSIn.Color = VSIn.Color;
}
