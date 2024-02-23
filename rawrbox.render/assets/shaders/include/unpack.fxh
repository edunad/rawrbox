#ifndef INCLUDED_PACK
	#define INCLUDED_PACK

    float4 packFloatToRgba(float _value) {
        const float4 shift = float4(256 * 256 * 256, 256 * 256, 256, 1.0);
        const float4 mask = float4(0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0);

        float4 comp = frac(_value * shift);
        comp -= comp.xxyz * mask;

        return comp;
    }
#endif

