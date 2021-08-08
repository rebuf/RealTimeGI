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
layout(binding = 5) uniform sampler2DShadow SunShadow;

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



vec3 SampleIrradianceVolume(in ivec3 GridCoord, in vec3 DiffCoord, in SurfaceData Surface, 
	in IrradianceVolumeData IrVolume, in samplerCubeArray Irradiance, in samplerCubeArray Radiance)
{
	// Compute 8 Neighbour Probes Coordinate...
	ivec3 ProbeOffset[8];
	ProbeOffset[0] = ivec3(0,           0,           0.0);
	ProbeOffset[1] = ivec3(DiffCoord.x, 0,           0.0);
	ProbeOffset[2] = ivec3(0,           DiffCoord.y, 0.0);
	ProbeOffset[3] = ivec3(DiffCoord.x, DiffCoord.y, 0.0);

	ProbeOffset[4] = ivec3(0,           0,           DiffCoord.z);
	ProbeOffset[5] = ivec3(DiffCoord.x, 0,           DiffCoord.z);
	ProbeOffset[6] = ivec3(0,           DiffCoord.y, DiffCoord.z);
	ProbeOffset[7] = ivec3(DiffCoord.x, DiffCoord.y, DiffCoord.z);

	// Compute Trilinear Factors...
	vec3 Probe0Pos = GetProbePos(GridCoord + ProbeOffset[0], IrVolume);
	vec3 Probe7Pos = GetProbePos(GridCoord + ProbeOffset[7], IrVolume);
	vec3 Delta =  Probe7Pos - Probe0Pos;
	vec3 Alpha = (Surface.P - Probe0Pos) / Delta;


	// Accumulated Results.
	vec3 IrValue = vec3(0.0);
	float IrAlpha = 0.0;

	// Sample 8 Neighbours and interpolate.
	for (int iP = 0; iP < 8; ++iP)
	{
		ivec3 ProbeGridCoord = GridCoord + ProbeOffset[iP];
		vec4 IrSample = SampleIrVolumeLayer(ProbeGridCoord, Surface, IrVolume, Irradiance, Radiance);

		// Apply Interpolation factors.
		vec3 ProbeAlpha = mix(1.0 - Alpha, Alpha, abs(ProbeOffset[iP]));
		IrSample.a *= ProbeAlpha.x * ProbeAlpha.y * ProbeAlpha.z;
		IrSample.a = max(IrSample.a, 0.00001);

		IrValue += IrSample.rgb * IrSample.a;
		IrAlpha += IrSample.a;
	}

	return IrValue / IrAlpha;
}


vec4 ComputeIrradianceVolume(in SurfaceData Surface, in IrradianceVolumeData IrVolume,
	in samplerCubeArray Irradiance, in samplerCubeArray Radiance)
{
	// Clip & Attinuate...
	vec3 Atten = vec3(0.0);
	vec3 AttenOffset = IrVolume.Extent * Atten;
	
	vec3 LocalP = Surface.P - (IrVolume.Start - AttenOffset);
	LocalP = LocalP / (IrVolume.Extent + AttenOffset * 2.0);

	if ( LocalP.x < 0.0 || LocalP.x > 1.0  
		|| LocalP.y < 0.0 || LocalP.y > 1.0 
		|| LocalP.z < 0.0 || LocalP.z > 1.0 )
		return vec4(0.0);

	Atten = max(Atten, 0.00001);
	LocalP.x = (LocalP.x < Atten.x) ? LocalP.x : 1.0 - LocalP.x;
	LocalP.y = (LocalP.y < Atten.y) ? LocalP.y : 1.0 - LocalP.y;
	LocalP.z = (LocalP.z < Atten.z) ? LocalP.z : 1.0 - LocalP.z;
	LocalP = LocalP / Atten;


	// Smooth Attenuation factor used for blending between volumes.
	vec3 IrSmooth = vec3(1.0);
	IrSmooth = smoothstep(0.0, 1.0, LocalP);


	// Lighting Surface using Irradiance Volume...
	ivec3 GridCoord = GetGridCoord(Surface.P, IrVolume);
	vec3 GridPos = GetProbePos(GridCoord, IrVolume);
	vec3 DiffCoord = sign(Surface.P - GridPos);

	vec3 IrValue = SampleIrradianceVolume(GridCoord, DiffCoord, Surface, IrVolume, Irradiance, Radiance);
	vec3 Kd = IrValue.rgb * Surface.Albedo;

	if ((inCommon.Mode & COMMON_MODE_REF_CAPTURE) != 0)
		return vec4(0.0);

	return vec4(IrValue.rgb, 1.0);
}


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

