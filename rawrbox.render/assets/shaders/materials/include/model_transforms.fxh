

#ifndef INCLUDED_MODEL_TRANSFORMS
    #ifdef INCLUDED_CAMERA
        #define INCLUDED_MODEL_TRANSFORMS

        struct TransformedData {
            float4 pos;
            float4 final;
        };

        // Snap vertex to achieve PSX look
        #ifdef TRANSFORM_PSX
            float4 PSXTransform(float4 vertex, float2 resolution) {
                float4 snappedPos = vertex;
                snappedPos.xyz = vertex.xyz / vertex.w;                         // convert to normalised device coordinates (NDC)
                snappedPos.xy = floor(resolution * snappedPos.xy) / resolution; // snap the vertex to the lower-resolution grid
                snappedPos.xyz *= vertex.w;                                     // convert back to projection-space

                return snappedPos;
            }
        #endif
        // ----------------------

        #ifdef TRANSFORM_BILLBOARD
            float4 billboardTransform(float4 vertex, float4 billboard) {
                float4 vOut = vertex;

                // TOOD: Lock X Y Z using billboard
                if(billboard.x != 0. || billboard.y != 0. || billboard.z != 0.) {
                    float3 right = float3(Camera.view[0][0], Camera.view[1][0], Camera.view[2][0]);
                    float3 up = float3(Camera.view[0][1], Camera.view[1][1], Camera.view[2][1]);

                    vOut = float4((right * vertex.x) + (up * vertex.y), 1.);
                }

                return vOut;
            }
        #endif

        #ifdef TRANSFORM_BONES
            float4 boneTransform(uint4 indices, float4 weight, float3 position) {
                float4x4 BoneTransform = float4x4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
                bool skinned = false;

                for (uint idx = 0; idx < MAX_BONES_PER_VERTEX; idx++) {
                    if (weight[idx] > 0.0) {
                        BoneTransform += Constants.bones[indices[idx]] * weight[idx];
                        skinned = true;
                    }
                }

                return skinned ? mul(BoneTransform, float4(position, 1.f)) : float4(position, 1.f);
            }
        #endif

        // Apply model transforms
        TransformedData applyPosTransforms(float4x4 proj, float4 a_position, float2 a_texcoord0) {
            TransformedData data;
            data.pos = a_position;

            // displacement mode
            #ifdef TRANSFORM_DISPLACEMENT
                if(DisplacementPower != 0. && DisplacementTexture != 0.) {
                    data.pos.y += g_Textures[DisplacementTexture].SampleLevel(g_Sampler, float3(a_texcoord0, 0), 0).x * DisplacementPower;
                }
            #endif
            // ----

            // Billboard mode
            #ifdef TRANSFORM_BILLBOARD
                data.pos = billboardTransform(data.pos, Billboard);
            #endif
            // ----

            // vertex_snap mode
            #ifdef TRANSFORM_PSX
                if(VertexSnap != 0.) {
                    data.final = PSXTransform(mul(data.pos, proj), ScreenSize / VertexSnap);
                } else {
                    data.final = mul(data.pos, proj);
                }
            #else
                data.final = mul(data.pos, proj);
            #endif
            // ----

            return data;
        }

        TransformedData applyPosTransforms(float4 a_position, float2 a_texcoord0) {
            return applyPosTransforms(Camera.worldViewProj, a_position, a_texcoord0);
        }

        TransformedData applyPosTransforms(float3 a_position, float2 a_texcoord0) {
            return applyPosTransforms(Camera.worldViewProj, float4(a_position, 1.0), a_texcoord0);
        }

        TransformedData applyPosTransforms(float4x4 proj, float3 a_position, float2 a_texcoord0) {
            return applyPosTransforms(proj, float4(a_position, 1.0), a_texcoord0);
        }
        // ----------------------
    #endif
#endif
