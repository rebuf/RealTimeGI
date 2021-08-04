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
layout(binding = 6) uniform samplerCube Irradiance;
layout(binding = 7) uniform samplerCube Radiance;

layout(push_constant) uniform Constants
{
	vec4 Position;
	vec4 Radius;
} inConstant;

#endif




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



#if defined(LIGHTING_PASS_IRRADIANCE_VOLUME)

int GetProbeIndex(ivec3 GridCoord, in IrradianceVolumeData IrVolume)
{
	return GridCoord.x + GridCoord.y * IrVolume.Count.x + GridCoord.z * (IrVolume.Count.x * IrVolume.Count.y);
}


vec3 GetProbePos(ivec3 GridCoord, in IrradianceVolumeData IrVolume)
{
	return vec3(GridCoord) * IrVolume.GridSize + IrVolume.GridSize * 0.5 + IrVolume.Start;
}



float SampleIrOcclusion(vec3 v, float ld, float layer, in samplerCubeArray Radiance) 
{ 
	float bias = 0.016; 
	
	float offset = 2.5;
	float numSamples = 3.0;
	float M1 = 0.0;
	float M2 = 0.0;
	
	for(float x = -offset; x < offset; x += offset / (numSamples * 0.5)) 
	{ 
		for(float y = -offset; y < offset; y += offset / (numSamples * 0.5))
		{
			for(float z = -offset; z < offset; z += offset / (numSamples * 0.5)) 
			{
				float s_depth = texture(Radiance, vec4(v + vec3(x, y, z), layer)).a; 

				M1 += s_depth;
				M2 += s_depth * s_depth;
			} 
		} 
	}

	M1 /= 27.0;
	M2 /= 27.0;


	float Ver = M2 - M1 * M1;
	ld -= bias;
	float g = ld - M1;

	return  g > 0.0 ? (Ver / (Ver + g * g)) : 1.0;
}


vec4 SampleIrVolumeLayer(in ivec3 GridCoord, out vec3 Probe0Pos, in SurfaceData Surface, 
	in IrradianceVolumeData IrVolume, in samplerCubeArray Irradiance, in samplerCubeArray Radiance)
{
	Probe0Pos = GetProbePos(GridCoord, IrVolume);
	GridCoord = clamp(GridCoord, ivec3(0), IrVolume.Count - 1);
	int Probe0Index = GetProbeIndex(GridCoord, IrVolume);
	float SampleRadius = IrVolume.GridLen * 1.2;
	vec3 Sample0 = LightProbeSampleRay(Probe0Pos, SampleRadius, Surface.P, Surface.N);
	vec4 Irradiance0 = texture(Irradiance, vec4(Sample0, Probe0Index));
	
	vec3 V = Surface.P - Probe0Pos;
	float Dist = length(V);
	float Occlusion = SampleIrOcclusion(V, Dist * 0.001, Probe0Index, Radiance);
	
	if (dot(V, Surface.N) > 0.0)
		Occlusion *= abs(dot(V, Surface.N) * 0.05);

	return vec4(Irradiance0.rgb, 1.0);
}



vec4 SampleIrradianceVolume(in ivec3 GridCoord, in vec3 DiffCoord, out vec3 Probe0Pos, in SurfaceData Surface, 
	in IrradianceVolumeData IrVolume, in samplerCubeArray Irradiance, in samplerCubeArray Radiance)
{
	ivec3 Probe0GridCoord = GridCoord + ivec3(0,           0,           DiffCoord.z);
	ivec3 Probe1GridCoord = GridCoord + ivec3(DiffCoord.x, 0,           DiffCoord.z);
	ivec3 Probe2GridCoord = GridCoord + ivec3(0,           DiffCoord.y, DiffCoord.z);
	ivec3 Probe3GridCoord = GridCoord + ivec3(DiffCoord.x, DiffCoord.y, DiffCoord.z);


	vec3 Probe1Pos, Probe2Pos, Probe3Pos;

	vec4 Irradiance0 = SampleIrVolumeLayer(Probe0GridCoord, Probe0Pos, Surface, IrVolume, Irradiance, Radiance);
	vec4 Irradiance1 = SampleIrVolumeLayer(Probe1GridCoord, Probe1Pos, Surface, IrVolume, Irradiance, Radiance);
	vec4 Irradiance2 = SampleIrVolumeLayer(Probe2GridCoord, Probe2Pos, Surface, IrVolume, Irradiance, Radiance);
	vec4 Irradiance3 = SampleIrVolumeLayer(Probe3GridCoord, Probe3Pos, Surface, IrVolume, Irradiance, Radiance);

//	Probe0Pos = GetProbePos(Probe0GridCoord, IrVolume);
//	vec3 Probe1Pos = GetProbePos(Probe1GridCoord, IrVolume);
//	vec3 Probe2Pos = GetProbePos(Probe2GridCoord, IrVolume);
//	vec3 Probe3Pos = GetProbePos(Probe3GridCoord, IrVolume);
//
//	Probe0GridCoord = clamp(Probe0GridCoord, ivec3(0), IrVolume.Count - 1);
//	Probe1GridCoord = clamp(Probe1GridCoord, ivec3(0), IrVolume.Count - 1);
//	Probe2GridCoord = clamp(Probe2GridCoord, ivec3(0), IrVolume.Count - 1);
//	Probe3GridCoord = clamp(Probe3GridCoord, ivec3(0), IrVolume.Count - 1);
//
//
//	int Probe0Index = GetProbeIndex(Probe0GridCoord, IrVolume);
//	int Probe1Index = GetProbeIndex(Probe1GridCoord, IrVolume);
//	int Probe2Index = GetProbeIndex(Probe2GridCoord, IrVolume);
//	int Probe3Index = GetProbeIndex(Probe3GridCoord, IrVolume);
//
//
//	float SampleRadius = IrVolume.GridLen * 1.2;
//
//	vec3 Sample0 = LightProbeSampleRay(Probe0Pos, SampleRadius, Surface.P, Surface.N);
//	vec3 Sample1 = LightProbeSampleRay(Probe1Pos, SampleRadius, Surface.P, Surface.N);
//	vec3 Sample2 = LightProbeSampleRay(Probe2Pos, SampleRadius, Surface.P, Surface.N);
//	vec3 Sample3 = LightProbeSampleRay(Probe3Pos, SampleRadius, Surface.P, Surface.N);
//
//	vec4 Irradiance0 = texture(Irradiance, vec4(Sample0, Probe0Index));
//	vec4 Irradiance1 = texture(Irradiance, vec4(Sample1, Probe1Index));
//	vec4 Irradiance2 = texture(Irradiance, vec4(Sample2, Probe2Index));
//	vec4 Irradiance3 = texture(Irradiance, vec4(Sample3, Probe3Index));

	vec2 Delta = Probe3Pos.xy - Probe0Pos.xy;
	vec2 Alpha = (Surface.P.xy - Probe0Pos.xy) / Delta;

	vec4 IrLerp0 = mix(Irradiance0, Irradiance1, Alpha.x);
	vec4 IrLerp1 = mix(Irradiance2, Irradiance3, Alpha.x);
	vec4 IrValue  = mix(IrLerp0, IrLerp1, Alpha.y);

	return IrValue;
}


vec4 VisualizeCubeMapArray(in samplerCubeArray Map, vec2 Coord, float layer)
{
	Coord.y = 1.0 - Coord.y;

	float Theta  = Coord.x * PI * 2.0;
	float Phi    = (Coord.y - 0.5) * PI;
	float CosPhi = cos(Phi);	

	vec3 n = vec3(CosPhi * cos(Theta), CosPhi * sin(Theta), sin(Phi));
	return texture(Map, vec4(n.xyz, layer));
}


vec4 ComputeIrradianceVolume(in SurfaceData Surface, in IrradianceVolumeData IrVolume,
	in samplerCubeArray Irradiance, in samplerCubeArray Radiance)
{
	// The Grid cell the surface point is in.
	ivec3 GridCoord = GetGridCoord(Surface.P, IrVolume);

	// Out of Volume Bounds?
	if ( GridCoord.x < 0 
	  || GridCoord.y < 0 
	  || GridCoord.z < 0 
	  || GridCoord.x > IrVolume.Count.x - 1 
	  || GridCoord.y > IrVolume.Count.y - 1 
	  || GridCoord.z > IrVolume.Count.z - 1 )
		return vec4(0.0);


	vec3 GridPos = GetProbePos(GridCoord, IrVolume);
	vec3 DiffCoord = sign(Surface.P - GridPos);

//	if (GetProbeIndex(GridCoord, IrVolume) != 4)
//		return vec4(0.0);

//	vec2 UV = (Surface.P.xy - GridPos.xy) / IrVolume.GridSize.xy;
//	UV += 0.5;
//
//	return VisualizeCubeMapArray(Irradiance, UV, GetProbeIndex(GridCoord, IrVolume));	

	vec3 Probe0Pos;
	vec3 Probe1Pos;

	vec4 IrLerp0 = SampleIrradianceVolume(GridCoord, vec3(DiffCoord.xy, 0.0), Probe0Pos, Surface, IrVolume, Irradiance, Radiance);
	vec4 IrLerp1 = SampleIrradianceVolume(GridCoord, DiffCoord, Probe1Pos, Surface, IrVolume, Irradiance, Radiance);


	float Delta = Probe1Pos.z - Probe0Pos.z;
	float Alpha = (Surface.P.z - Probe0Pos.z) / Delta;
	vec4 IrValue  = mix(IrLerp0, IrLerp1, Alpha);

	vec3 Kd = IrValue.rgb * Surface.Albedo;
	
	return vec4(IrValue.rgb, 1.0);
	//return vec4(Kd, 1.0);
}

#endif




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

