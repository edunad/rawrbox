// Based off https://github.com/simco50/D3D12_Research <3

#ifdef INCLUDED_DECAL_UNIFORMS
#ifdef READ_DECALS
#ifdef READ_CLUSTER_DATA_GRID

    #ifndef INCLUDED_DECALS
        #define INCLUDED_DECALS

        void ApplyDecals(in uint decalBucket, uint bucketIndex, float4 worldPosition, float3 ddxPos, float3 ddyPos, inout float4 baseColor, inout float4 roughtness) {
            while(decalBucket) {
                uint bitIndex = firstbitlow(decalBucket);
                decalBucket ^= 1u << bitIndex;

                // Apply decal ------------
                Decal decal = Decals[bitIndex + bucketIndex * CLUSTERS_Z];

                float4 dPos = mul(worldPosition, decal.worldToLocal);
                float3 decalTexCoord = dPos.xyz * float3(0.5f, -0.5f, 0.5f) + 0.5f;
                float4 decalAccumulation = 0;

                if(all(decalTexCoord >= 0.0) && all(decalTexCoord <= 1.0)) {
                    float2 decalDx = mul(ddxPos, (float3x3)decal.worldToLocal).xy;
                    float2 decalDy = mul(ddyPos, (float3x3)decal.worldToLocal).xy;

                    float4 decalColor = g_Textures[decal.data.x].SampleGrad(g_Textures_sampler, float3(decalTexCoord.xy, decal.data.y), decalDx, decalDy, 0) * decal.color;
                    float edge = 1 - pow(saturate(abs(dPos.z)), 8);
                    decalColor.a *= edge;

                    decalAccumulation.rgb = (1 - decalColor.a) * decalAccumulation.rgb + decalColor.a * decalColor.rgb;
                    decalAccumulation.a += decalColor.a;

                    baseColor.rgb = lerp(baseColor.rgb, decalAccumulation.rgb, decalAccumulation.a);
                    roughtness.rgb = lerp(roughtness.rgb, 0.9f, decalAccumulation.a);
                }
                // ------------------------
            }
        }
    #endif

#endif
#endif
#endif
