#ifdef INCLUDED_DECAL_UNIFORMS
#ifdef READ_DECALS
#ifdef READ_CLUSTER_DATA_GRID

    #ifndef INCLUDED_DECALS
        #define INCLUDED_DECALS

        Decal GetDecal(uint index) {
            return Decals[NonUniformResourceIndex(index)];
        }

        void ApplyDecals(uint decalBucket, uint bucketIndex, float4 worldPosition, float3 ddxPos, float3 ddyPos, inout float4 baseColor, inout float4 roughtness) {
            uint bucket = decalBucket;
            if(TOTAL_DECALS == 0) return;

            while(bucket) {
                uint bitIndex = firstbitlow(bucket);
                bucket ^= 1u << bitIndex;

                // Apply decal ------------
                uint index = bitIndex + bucketIndex * CLUSTERS_Z;
                if(index > TOTAL_DECALS) break;

                Decal decal = GetDecal(index);

                float4 dPos = mul(worldPosition, decal.worldToLocal);
                float3 decalTexCoord = dPos.xyz * float3(0.5f, -0.5f, 0.5f) + 0.5f;
                float4 decalAccumulation = 0;

                float3x3 wLocal = (float3x3)decal.worldToLocal;

                // Back-face culling check
                float3 viewDir = normalize(worldPosition.xyz - Camera.pos.xyz);
                float3 decalNormal = mul(float3(0, 0, 1), wLocal); // TODO: ADD NORMAL TO DECAL?

                if(dot(decalNormal, viewDir) < 0 && all(decalTexCoord >= 0.0) && all(decalTexCoord <= 1.0)) {
                    float2 decalDx = mul(ddxPos, wLocal).xy;
                    float2 decalDy = mul(ddyPos, wLocal).xy;

                    float4 decalColor = g_Textures[decal.data.x].SampleGrad(g_Sampler, float3(decalTexCoord.xy, decal.data.y), decalDx, decalDy, 0) * decal.color;
                    float edge = 1. - pow(saturate(abs(dPos.z)), 8.);
                    decalColor.a *= edge;

                    decalAccumulation.rgb = (1. - decalColor.a) * decalAccumulation.rgb + decalColor.a * decalColor.rgb;
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
