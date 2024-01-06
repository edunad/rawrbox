
#ifndef INCLUDED_CAMERA
    #define INCLUDED_CAMERA

    struct CameraStruct {
        float4x4 view;
        float4x4 viewInv;

        float4x4 proj;
        float4x4 projInv;

        float4x4 viewProj;
        float4x4 viewProjInv;

        float4x4 world;
        float4x4 worldViewProj;

        float4   nearFar;

        int4     viewport;
        float4   viewportInv;

        float4   cameraPos;
        float4   cameraAngle;

        // GRID ---
        float4   gridParams;
        // -----
    };

    ConstantBuffer<CameraStruct> Camera;

    // UTILS -----------------
    uint GetSliceFromDepth(float depth) {
        return floor(log(depth) * Camera.gridParams.x - Camera.gridParams.y);
    }

    float LinearizeDepth(float z, float near, float far) {
        return near / (near + z * (far - near));
    }

    float3 ViewPositionFromDepth(float2 uv, float depth, float2 nearFar, float4x4 invProj) {
        float4 clip = float4(float2(uv.x, 1.0f - uv.y) * 2.0f - 1.0f, 0.0f, 1.0f) * nearFar.y;
        float3 viewRay = mul(clip, invProj).xyz;

        return viewRay * LinearizeDepth(depth, nearFar.x, nearFar.y);
    }

    float3 ScreenToView(float4 screen, float4 invScreenSize, float2 nearFar, float4x4 invProj) {
        float2 screenNormalized = screen.xy * invScreenSize.zw;
        return ViewPositionFromDepth(screenNormalized, screen.z, nearFar, invProj);
    }
    // -----------------------
#endif
