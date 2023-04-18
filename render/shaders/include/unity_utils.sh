
#ifndef INCLUDED_UNITY_UTILS
#define INCLUDED_UNITY_UTILS

// Unity built-in shader source. Copyright (c) 2016 Unity Technologies. MIT license (see license.txt)
// Adapted from https://github.com/TwoTailsGames/Unity-Built-in-Shaders/blob/master/CGIncludes/UnityCG.cginc

// Convert rgb to luminance
// with rgb in linear space with sRGB primaries and D65 white point
float LinearRgbToLuminance(vec3 linearRgb) {
	return dot(linearRgb, vec3(0.2126729f, 0.7151522f, 0.0721750f));
}

vec3 GammaToLinearSpace(vec3 sRGB) {
	// Approximate version from http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html?m=1
	return sRGB * (sRGB * (sRGB * 0.305306011 + 0.682171111) + 0.012522878);
}

/*
Copyright (c) 2016 Unity Technologies

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*
*/
#endif
