#ifndef INCLUDED_MATH
	#define INCLUDED_MATH

    static const float FLT_MIN  = 1.175494351e-38F;
    static const float FLT_MAX  = 3.402823466e+38F;

    static const float GRAVITY  = -9.81F;

    static const float PI      	= 3.14159265358979323846;
    static const float INV_PI   = 0.31830988618379067154;
    static const float INV_2PI  = 0.15915494309189533577;
    static const float INV_4PI  = 0.07957747154594766788;
    static const float PI_DIV_2 = 1.57079632679489661923;
    static const float PI_DIV_4 = 0.78539816339744830961;
    static const float SQRT_2   = 1.41421356237309504880;

    static const uint INVALID_HANDLE = 0xFFFFFFFF;

    static const float3x3 IDENTITY_MATRIX_3 = float3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);
    static const float4x4 IDENTITY_MATRIX_4 = float4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

    float Square(float x) {
        return x * x;
    }

    float Pow4(float x) {
        float xx = x * x;
        return xx * xx;
    }

    float Pow5(float x) {
        float xx = x * x;
        return xx * xx * x;
    }

    // UTILS -----------------
	uint Flatten2D(uint2 index, uint dimensionsX) {
		return index.x + index.y * dimensionsX;
	}

	uint Flatten3D(uint3 index, uint2 dimensionsXY) {
		return index.x + index.y * dimensionsXY.x + index.z * dimensionsXY.x * dimensionsXY.y;
	}

	uint2 UnFlatten2D(uint index, uint dimensionsX) {
		return uint2(index % dimensionsX, index / dimensionsX);
	}

	uint3 UnFlatten3D(uint index, uint2 dimensionsXY) {
		uint3 outIndex;
		outIndex.z = index / (dimensionsXY.x * dimensionsXY.y);
		index -= (outIndex.z * dimensionsXY.x * dimensionsXY.y);
		outIndex.y = index / dimensionsXY.x;
		outIndex.x = index % dimensionsXY.x;
		return outIndex;
	}
    // -----------------------
#endif

