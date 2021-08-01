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
#include "CommonLighting.glsl"




// Vertex Input...
layout(location = 0) in vec2 TexCoord;
layout(location = 1) in vec2 TargetTexCoord; // The texture coordinate for sampling render targets.


// G-Buffer Input...
layout(binding = 1) uniform sampler2D gAlbedo;
layout(binding = 2) uniform sampler2D gBRDF;
layout(binding = 3) uniform sampler2D gNormal;
layout(binding = 4) uniform sampler2D gDepth;


#if defined(LIGHTING_PASS_SUN_LIGHT)
// Sun Light Shadow Map
layout(binding = 5) uniform sampler2D SunShadow;

layout(push_constant) uniform Constants
{
	mat4 SunTransform;
} inConstant;

#endif



#if defined(LIGHTING_PASS_LIGHT_PROBE)
// Sun Light Shadow Map
layout(binding = 6) uniform samplerCube Irradiance;
layout(binding = 7) uniform samplerCube Radiance;

layout(push_constant) uniform Constants
{
	vec4 Position;
	vec4 Radius;
} inConstant;

#endif



// Output...
layout(location = 0) out vec4 FragColor;



void main()
{
	vec4 Albedo = texture(gAlbedo, TargetTexCoord);
	vec4 BRDF = texture(gBRDF, TargetTexCoord);
	vec3 Normal = normalize(texture(gNormal, TargetTexCoord).xyz);
	float Depth = texture(gDepth, TargetTexCoord).r;


	// Surface...
	SurfaceData Surface;
	Surface.P = ComputeWorldPos(Depth, TexCoord);
	Surface.N = Normal;
	Surface.V = normalize(inCommon.ViewPos - Surface.P);

	Surface.Albedo = Albedo.rgb;

	vec3 Lighting = vec3(0.0);

#if defined(LIGHTING_PASS_SUN_LIGHT)
	Lighting = ComputeSunLight(Surface, SunShadow, inConstant.SunTransform);
	FragColor.rgb = Lighting;
	FragColor.a = 1.0; 
#elif defined(LIGHTING_PASS_LIGHT_PROBE)
	FragColor = ComputeIBLight(Surface, inConstant.Position.xyz, inConstant.Radius.x, Irradiance, Radiance);
#endif

}

