#ifndef INCLUDED_LIGHT_UNIFORMS
    #define INCLUDED_LIGHT_UNIFORMS

    struct LightConstantsStruct {
        // Light ---------
        uint4 lightSettings;
        // ------

        // Ambient ---
        float4 ambientColor;
        // -----

        // Fog ---
        float4 fogColor;
        float4 fogSettings;
        // -----
    };

    ConstantBuffer<LightConstantsStruct> LightConstants;

    struct Light {
        float4 position;
        float4 direction;

        float3 color;
        float  intensity;

        // -------
        float  radius;
        float  penumbra;
        float  umbra;

        uint   type;
        // -------
    };

    // Aka sun
    struct DirectionalLight {
        float4 direction;
        float4 radiance;
    };


    #define LIGHT_POINT       1
    #define LIGHT_SPOT        2
    #define LIGHT_DIRECTIONAL 3

    #define TOTAL_LIGHTS LightConstants.lightSettings.y
#endif
