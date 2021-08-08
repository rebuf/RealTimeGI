// Copyright (c) 2021 Ammar Herzallah
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.



#version 450
#extension GL_ARB_separate_shader_objects : enable



#include "Common.glsl"


#define LUMA_FACTORS vec3(0.299, 0.587, 0.114)



// Vertex Input...
layout(location = 0) in vec2 TexCoord;
layout(location = 1) in vec2 TargetTexCoord; // The texture coordinate for sampling render targets.


// Input...
layout(binding = 1) uniform sampler2D RenderTarget0;


// Output...
layout(location = 0) out vec4 FragColor;





// Tone Mapping Uncharted2:
//    - John Hable, Uncharted 2: HDR Lighting, GDC, https://www.gdcvault.com/play/1012351/Uncharted-2-HDR
//
vec3 Tonemap(vec3 Color) 
{

// Approxiate Filmic
#define UNCHARTED_TONE_OP(x) ( ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F )

	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	
	
	// Exposure Bias.
	float Exp = 3.5;
	
	A = 0.35;
	C = 0.08;
	
	
	// Apply Tone Mapping...
	vec3 Tone = UNCHARTED_TONE_OP(Exp * Color);
	Tone = Tone / UNCHARTED_TONE_OP(W);
	
	// Gamma.
	return pow(Tone, vec3(0.45454545));
}





void main()
{
	FragColor = texture(RenderTarget0, TargetTexCoord);

	FragColor.rgb = Tonemap(FragColor.rgb);

	FragColor.a =  dot(FragColor.rgb, LUMA_FACTORS);
}

