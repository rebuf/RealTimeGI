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


precision highp float;




#include "Common.glsl"





// Geom Input...
layout(location = 0) in vec2 TexCoord;
layout(location = 1) in vec2 TargetTexCoord; // The texture coordinate for sampling render targets.

// Input...
layout(binding = 1) uniform sampler2D CaptureRender;
layout(binding = 2) uniform sampler2D CaptureDetphRender;


// Output...
layout(location = 0) out vec4 FragColor;



void main()
{
	FragColor.rgb = texture(CaptureRender, TargetTexCoord).rgb;

	float Depth = texture(CaptureDetphRender, TargetTexCoord).r;
	vec3 WorldPos = ComputeWorldPos(Depth, TexCoord);
	vec3 Center = ComputeWorldPos(0.0, TexCoord);
	Depth = length(WorldPos - Center);

	FragColor.a = clamp(Depth * 0.001, 0.0, 1.0);
}

