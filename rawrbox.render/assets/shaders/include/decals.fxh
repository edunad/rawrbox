
#ifndef INCLUDED_DECALS
    #define INCLUDED_DECALS

    Texture2DArray g_DecalTexture;
    SamplerState   g_DecalTexture_sampler;

    cbuffer Decals {
        uint4 g_DecalSettings;
    };

    struct Decal {
        float4x4 worldToLocal;
        uint4 data;
        float4 color;
    };

    #define TOTAL_DECALS g_DecalSettings.x

    StructuredBuffer<Decal> g_Decals; // Read-only

    void ApplyDecals(float4 worldPosition, inout float4 baseColor, inout float4 roughtness) {
        for(uint i = 0; i < TOTAL_DECALS; ++i) {
            Decal decal = g_Decals[i];

            float4 dPos = mul(worldPosition, decal.worldToLocal);
            float3 decalTexCoord = dPos.xyz * float3(0.5f, -0.5f, 0.5f) + 0.5f;
            float4 decalAccumulation = 0;

            if(all(decalTexCoord >= 0.0) && all(decalTexCoord <= 1.0)) {
                float3 ddxPos = ddx(worldPosition.xyz);
                float3 ddyPos = ddy(worldPosition.xyz);
                float2 decalDx = mul(ddxPos, (float3x3)decal.worldToLocal).xy;
                float2 decalDy = mul(ddyPos, (float3x3)decal.worldToLocal).xy;

                float4 decalColor = g_DecalTexture.Sample(g_DecalTexture_sampler, float3(decalTexCoord.xy, decal.data.x)) * decal.color;
                float edge = 1 - pow(saturate(abs(dPos.z)), 8);
                decalColor.a *= edge;

                decalAccumulation.rgb = (1 - decalColor.a) * decalAccumulation.rgb + decalColor.a * decalColor.rgb;
			    decalAccumulation.a += decalColor.a;

                baseColor.rgb = lerp(baseColor.rgb, decalAccumulation.rgb, decalAccumulation.a);
                roughtness.rgb = lerp(roughtness.rgb, 0.9f, decalAccumulation.a);
            }
        }
    }
#endif
