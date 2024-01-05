
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
#endif
