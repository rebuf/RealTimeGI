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

#if defined(LIGHTING_PASS_LIGHT_PROBE)
#include "LightProbe.glsl"
#elif defined(LIGHTING_PASS_IRRADIANCE_VOLUME)
#include "IrradianceVolume.glsl"
#endif



// Vertex Input...
layout(location = 0) in vec2 TexCoord;
layout(location = 1) in vec2 TargetTexCoord; // The texture coordinate for sampling render targets.


// G-Buffer Input...
layout(binding = 1) uniform sampler2D gAlbedo;
layout(binding = 2) uniform sampler2D gBRDF;
layout(binding = 3) uniform sampler2D gNormal;
layout(binding = 4) uniform sampler2D gDepth;



// SUN_LIGHT Input
#if defined(LIGHTING_PASS_SUN_LIGHT)
layout(binding = 5) uniform sampler2D SunShadow;

layout(push_constant) uniform Constants
{
	mat4 SunTransform;
} inConstant;

#endif



// LIGHT_PROBE Input
#if defined(LIGHTING_PASS_LIGHT_PROBE)
layout(binding = 6) uniform samplerCube Irradiance;
layout(binding = 7) uniform samplerCube Radiance;

layout(push_constant) uniform Constants
{
	vec4 Position;
	vec4 Radius;
} inConstant;

#endif




// IRRADIANCE_VOLUME Input
#if defined(LIGHTING_PASS_IRRADIANCE_VOLUME)
layout(binding = 6) uniform samplerCubeArray IrradianceArray;
layout(binding = 7) uniform samplerCubeArray RadianceArray;

layout(push_constant) uniform Constants
{
	vec4 Start;
	vec4 Extent;
	vec4 Count;
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
	Surface.NDotV = max(dot(Surface.N, Surface.V), 0.0001);

	Surface.Albedo = Albedo.rgb;
	Surface.Roughness = BRDF.r;
	Surface.Metallic = BRDF.g;
	Surface.Specular = 1.0;

	// Base Reflective Index, 0.04 similar to UE4
	Surface.F0 = mix(vec3(0.04), Surface.Albedo, Surface.Metallic);


	// lighting...
	vec3 Lighting = vec3(0.0);

#if defined(LIGHTING_PASS_SUN_LIGHT)
	Lighting = ComputeSunLight(Surface, SunShadow, inConstant.SunTransform);
	FragColor.rgb = Lighting;
	FragColor.a = 1.0; 
#elif defined(LIGHTING_PASS_LIGHT_PROBE)
	FragColor = ComputeLightProbe(Surface, inConstant.Position.xyz, inConstant.Radius.x, Irradiance, Radiance);
#elif defined(LIGHTING_PASS_IRRADIANCE_VOLUME)
	IrradianceVolumeData IrVolume;
	IrVolume.Start = inConstant.Start.xyz;
	IrVolume.Extent = inConstant.Extent.xyz;
	IrVolume.Count = ivec3(inConstant.Count.xyz);
	IrVolume.GridSize = ivec3(IrVolume.Extent.xyz / inConstant.Count.xyz);
	IrVolume.GridLen = length(IrVolume.GridSize);

	FragColor = ComputeIrradianceVolume(Surface, IrVolume, IrradianceArray, RadianceArray);
#endif

}

