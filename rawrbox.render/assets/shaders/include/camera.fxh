
#ifndef INCLUDED_CAMERA
    #define INCLUDED_CAMERA
    cbuffer Camera {
        float4x4 g_view;
        float4x4 g_viewInv;

        float4x4 g_proj;
        float4x4 g_projInv;

        float4x4 g_viewProj;
        float4x4 g_viewProjInv;

        float4x4 g_model;
        float4x4 g_worldViewProj;

        float2   g_nearFar;
        int2     g_screenSize;

        float4   g_cameraPos;
        float4   g_cameraAngle;
    };
#endif
