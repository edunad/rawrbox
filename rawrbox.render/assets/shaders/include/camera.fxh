
#ifndef INCLUDED_CAMERA
    #define INCLUDED_CAMERA

    struct CameraStruct {
        float4x4 view;

        float4x4 world;
        float4x4 worldViewProj;

        float4   pos;
    };

    // Data that never / very rarelly changes
    struct StaticCameraStruct {
        float4x4 proj;
        float4x4 projInv;

        float4   viewport;
        float2   gridParams;
    };

    ConstantBuffer<CameraStruct> Camera;
    ConstantBuffer<StaticCameraStruct> SCamera;

    #define ScreenSize SCamera.viewport.zw
    #define NearFar SCamera.viewport.xy
    #define px (float2(1.0, 1.0) / ScreenSize)

    // UTILS -----------------
    uint GetSliceFromDepth(float depth) {
        return floor(log(depth) * SCamera.gridParams.x - SCamera.gridParams.y);
    }

    float LinearizeDepth(float z, float near, float far) {
        return near / (near + z * (far - near));
    }

    float3 ViewPositionFromDepth(float2 uv, float depth, float2 nearFar, float4x4 invProj) {
        float4 clip = float4(float2(uv.x, 1.0f - uv.y) * 2.0f - 1.0f, 0.0f, 1.0f) * nearFar.y;
        float3 viewRay = mul(clip, invProj).xyz;

        return viewRay * LinearizeDepth(depth, nearFar.x, nearFar.y);
    }

    float3 WorldPositionFromDepth(float2 uv, float depth, float4x4 invProj) {
        float4 clip = float4(float2(uv.x, 1.0f - uv.y) * 2.0f - 1.0f, depth, 1.0f);
        float4 world = mul(clip, invProj);

        return world.xyz / world.w;
    }

    float3 ScreenToView(float3 screen, float2 invScreenSize, float2 nearFar, float4x4 invProj) {
        float2 screenNormalized = screen.xy * invScreenSize;
        return ViewPositionFromDepth(screenNormalized, screen.z, nearFar, invProj);
    }
    // -----------------------
#endif
