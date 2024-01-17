
#ifndef INCLUDED_CAMERA
    #define INCLUDED_CAMERA

    struct CameraStruct {
        float4x4 view;

        float4x4 proj;
        float4x4 projInv;

        float4x4 world;
        float4x4 worldViewProj;

        float4   viewport;
        float4   pos;

        float2   gridParams;
    };

    ConstantBuffer<CameraStruct> Camera;

    #define ScreenSize Camera.viewport.zw
    #define NearFar Camera.viewport.xy
    #define px (float2(1.0, 1.0) / ScreenSize)

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

    float3 ScreenToView(float3 screen, float2 invScreenSize, float2 nearFar, float4x4 invProj) {
        float2 screenNormalized = screen * invScreenSize;
        return ViewPositionFromDepth(screenNormalized, screen.z, nearFar, invProj);
    }
    // -----------------------
#endif
