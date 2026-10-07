#ifndef INCLUDED_COLOR_CORRECTION
#define INCLUDED_COLOR_CORRECTION

// From https://github.com/DiligentGraphics/DiligentCore
float toLinear(float x) {
	return x <= 0.04045F ? x / 12.92F : pow(abs((x + 0.055F) / 1.055F), 2.4F);
}

// http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
float fastToLinear(float x) {
	return x * (x * (x * 0.305306011f + 0.682171111f) + 0.012522878f);
}

float3 toLinear(float3 rgb) {
	rgb.x = toLinear(rgb.x);
	rgb.y = toLinear(rgb.y);
	rgb.z = toLinear(rgb.z);

	return rgb;
}

float4 toLinear(float4 rgb) {
	rgb.x = toLinear(rgb.x);
	rgb.y = toLinear(rgb.y);
	rgb.z = toLinear(rgb.z);
	rgb.w = rgb.w;
	return rgb;
}

float toSRGB(float x) {
	return x <= 0.0031308F ? x * 12.92F : 1.055F * pow(abs(x), 1.0F / 2.4F) - 0.055F;
}

float3 toSRGB(float3 rgb) {
	rgb.x = toSRGB(rgb.x);
	rgb.y = toSRGB(rgb.y);
	rgb.z = toSRGB(rgb.z);
	return rgb;
}
#endif
